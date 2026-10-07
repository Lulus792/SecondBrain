#include "desktop.h"
#include "platform.h"
#include "icons.h"
#include "version.h"
#include "notices.h"
#include "markdown.h"
#include "document.h"
#include "inline.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
static void backup_poll(SBDesktop *d);
static void accessible_actions(SBDesktop *d);
static void accessible_publish(SBDesktop *d);
static void style_update(SBDesktop *d,bool force) {
    d->system_style=sb_system_style_snapshot(d->style_monitor,force);
    SBStyleChoice effective=sb_style_resolve(d->requested_style,d->system_style);
    SBStyleChoice old=d->applied_style;
    if (force || effective.dark!=old.dark || effective.contrast!=old.contrast) {
        d->ui.contrast=effective.contrast; sb_ui_theme(&d->ui,effective.dark);
    }
    if (force || effective.solid!=old.solid) d->solid=effective.solid;
    if (force || effective.motion!=old.motion) d->reduced_motion=effective.motion;
    d->applied_style=effective;
}
void sb_desktop_set_style(SBDesktop *d,SBStyleChoice style) { d->requested_style=style; style_update(d,true); }
static bool modal_reader(const SBDesktop *d) { return d->form==SB_FORM_CONTEXT || d->form==SB_FORM_NOTICE_TEXT; }
static uint64_t accessible_context(const SBDesktop *d) {
    return sb_hash(d->model.project.root,strlen(d->model.project.root))^sb_hash(d->model.source_path,strlen(d->model.source_path))^
        ((uint64_t)d->model.generation<<24)^((uint64_t)d->form<<8)^((d->form==SB_FORM_NOTICE_TEXT ? (uint64_t)d->notice_index : 0)<<56)^(uint64_t)d->model.guard;
}

static const char *sections[] = {"all", "overview", "knowledge", "inbox", "journal", "archive"};
static const char *section_names[] = {"Alle Dokumente", "Orientierung", "Wissen", "Eingang", "Übergaben", "Archiv"};
static const char *new_sections[] = {"knowledge", "inbox", "journal"};
static const char *new_names[] = {"Wissen", "Eingang", "Übergaben"};

static void compact_label(SBDesktop *d, const char *text, char *out, size_t capacity, float width);
static void glass(SBDesktop *d, struct nk_rect r, float radius);

/* Motion uses elapsed time, so trackpads and keyboard repeats can retarget it. */
static float approach(float current, float destination, float factor, float threshold) {
    float value = current + (destination-current)*factor;
    return fabsf(destination-value) < threshold ? destination : value;
}
void sb_desktop_tick(SBDesktop *d, float seconds) {
    style_update(d,false);
    accessible_actions(d);
    backup_poll(d);
    d->seconds = fmaxf(0, fminf(seconds, 0.05f));
    float factor = d->reduced_motion ? 1 : 1-expf(-d->seconds/0.085f);
    d->view_yaw = approach(d->view_yaw,d->yaw,factor,0.0001f);
    d->view_pitch = approach(d->view_pitch,d->pitch,factor,0.0001f);
    d->view_zoom = approach(d->view_zoom,d->zoom,factor,0.0001f);
    d->view_pan_x = approach(d->view_pan_x,d->pan_x,factor,0.05f);
    d->view_pan_y = approach(d->view_pan_y,d->pan_y,factor,0.05f);
    d->flight=d->reduced_motion ? 1 : fminf(1,d->flight+d->seconds/0.72f);
    float q=d->flight*d->flight*d->flight*(d->flight*(d->flight*6-15)+10);
    d->focus_x=d->flight_from[0]+(d->flight_to[0]-d->flight_from[0])*q;
    d->focus_y=d->flight_from[1]+(d->flight_to[1]-d->flight_from[1])*q;
    d->focus_z=d->flight_from[2]+(d->flight_to[2]-d->flight_from[2])*q+(d->flight<1 ? 70*sinf(d->flight*3.14159265f) : 0);

}
bool sb_desktop_animating(const SBDesktop *d) {
    if (d->backup) return true;
    if (d->view_yaw != d->yaw || d->view_pitch != d->pitch || d->view_zoom != d->zoom ||
        d->view_pan_x != d->pan_x || d->view_pan_y != d->pan_y) return true;
    if (d->flight<1 || (d->map_ready && (fabsf(d->map_cx-d->map_target_cx)>0.05f || fabsf(d->map_cy-d->map_target_cy)>0.05f || fabsf(d->map_unit-d->map_target_unit)>0.0001f))) return true;
    for (unsigned i=0;i<4;++i) if (d->scrolling[i].active || d->scrolling[i].pending || fabsf(d->scrolling[i].elastic)>0.1f) return true;
    return false;
}
static void smooth_scroll(SBDesktop *d, unsigned slot, nk_uint *offset) {
    SBScroll *s = &d->scrolling[slot]; s->used=true;
    /* Scrollbar dragging, focus reveal and Nuklear's boundary clamp take priority. */
    if (!s->ready || *offset != s->applied) {
        s->position = s->destination = (float)*offset; s->active = false; s->ready = true;
    }
    if (s->pending) {
        if ((s->destination-s->position)*s->pending < 0) s->destination = s->position;
        float requested=s->destination+s->pending;
        s->destination=fmaxf(0,requested);
        if (s->measured) s->destination=fminf(s->maximum,s->destination);
        if (!d->reduced_motion && slot!=1 && s->maximum>0) s->elastic=fmaxf(-32*d->ui.scale,fminf(32*d->ui.scale,s->elastic+(requested-s->destination)*0.14f));
        s->pending=0; s->active=true;
    }
    float factor = d->reduced_motion ? 1 : 1-expf(-d->seconds/0.065f);
    s->position = approach(s->position,s->destination,factor,0.25f);
    *offset = s->applied = (nk_uint)roundf(s->position);
    s->active = s->position != s->destination;
    s->elastic=d->reduced_motion ? 0 : approach(s->elastic,0,1-expf(-d->seconds/0.11f),0.1f);
    if (slot!=1) d->ui.ctx->current->layout->at_y-=s->elastic;
}
static void scroll_gutter(SBDesktop *d) {
    /* Keep the overlay thumb and its pointer track out of content/focus rings.
       Reserve the same width before and after overflow to avoid reflow jumps. */
    struct nk_panel *panel=d->ui.ctx->current->layout;
    panel->bounds.w=fmaxf(1,panel->bounds.w-(10+6*d->ui.scale));
}
static void scroll_measure(SBDesktop *d, unsigned slot) {
    struct nk_panel *p=d->ui.ctx->current->layout;
    SBScroll *s=&d->scrolling[slot];
    p->at_y+=s->elastic;
    float maximum=floorf(fmaxf(0,p->at_y+p->row.height-p->bounds.y-p->bounds.h));
    /* Fractional rubber-band placement cannot change the content's size. */
    if (!s->measured || s->width!=p->bounds.w || s->height!=p->bounds.h || fabsf(maximum-s->maximum)>1)
        s->maximum=maximum;
    s->width=p->bounds.w; s->height=p->bounds.h; s->measured=true;
    s->destination=fminf(s->destination,s->maximum);
    if (*p->offset_y>s->maximum) *p->offset_y=(nk_uint)s->maximum;
    if (s->maximum<=0) { s->track=nk_rect(0,0,0,0); return; }
    float h=p->clip.h;
    float length=fmaxf(22*d->ui.scale,h*h/(h+s->maximum));
    length=fminf(h,length);
    float y=p->clip.y+(h-length)*fminf(1,(float)*p->offset_y/s->maximum);
    float squeeze=fminf(length/3,fabsf(s->elastic)*0.4f);
    if (s->elastic>0) y+=squeeze;
    s->track=nk_rect(p->clip.x+p->clip.w-10,p->clip.y,10,h);
    s->thumb=nk_rect(s->track.x+4,y,4,length-squeeze);
    struct nk_color ink=d->ui.dark ? nk_rgba(175,194,220,160) : nk_rgba(70,90,115,165);
    nk_fill_rect(nk_window_get_canvas(d->ui.ctx),s->thumb,2,ink);
}
static unsigned scroll_slot(SBDesktop *d, float x, float y, bool pointer) {
    if (d->model.guard) return 4;
    if (d->form != SB_FORM_NONE) {
        if (pointer) {
            int w,h; SDL_GetWindowSize(d->ui.window,&w,&h);
            float pw=fminf(w-40,580*d->ui.scale), ph=fminf(h-40,680*d->ui.scale);
            if (x < (w-pw)/2 || x >= (w+pw)/2 || y < (h-ph)/2 || y >= (h+ph)/2) return 4;
        }
        return modal_reader(d) ? 2 : 3;
    }
    if (!d->model.has_project) {
        if (pointer) {
            int w,h; SDL_GetWindowSize(d->ui.window,&w,&h);
            float cw=fminf(w-36.0f,620*d->ui.scale),ch=fminf(h-36.0f,520*d->ui.scale);
            if (x<(w-cw)/2 || x>=(w+cw)/2 || y<(h-ch)/2 || y>=(h+ch)/2) return 4;
        }
        return 2;
    }
    if (pointer) {
        for (size_t i=0;i<d->target_count;++i) {
            SBTarget *item = &d->targets[i];
            if (x >= item->bounds.x && y >= item->bounds.y && x < item->bounds.x+item->bounds.w && y < item->bounds.y+item->bounds.h) {
                if (!strcmp(item->id,"reader")) return 0;
                if (!strcmp(item->id,"editor")) return 1;
            }
        }
    } else {
        if (!strcmp(d->focus,"reader") || !strncmp(d->focus,"link:",5)) return 0;
    }
    if (pointer) {
        struct nk_window *window=nk_window_find(d->ui.ctx,"Detail");
        if (!d->card || !window || x < window->bounds.x || y < window->bounds.y ||
            x >= window->bounds.x+window->bounds.w || y >= window->bounds.y+window->bounds.h) return 4;
    } else {
        bool document=false;
        for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->focus,d->targets[i].id)) document=d->targets[i].group==2;
        if (!document) return 4;
    }
    return 2;
}

static SBTarget *target_add(SBDesktop *d, const char *id, struct nk_rect rect, SBFocusKind kind, int group) {
    if (d->target_count == d->target_capacity) {
        size_t capacity = d->target_capacity ? d->target_capacity * 2 : 64;
        SBTarget *items = realloc(d->targets,capacity*sizeof(*items));
        if (!items) { d->message = sb_error(SB_MEMORY,"Kein Speicher für Tastaturziele."); return NULL; }
        d->targets = items; d->target_capacity = capacity;
    }
    SBTarget *item = &d->targets[d->target_count++];
    memset(item,0,sizeof(*item));
    snprintf(item->id,sizeof(item->id),"%s",id);
    snprintf(item->label,sizeof(item->label),"%s",id);
    item->bounds = rect; item->kind = kind; item->group = group; item->order=d->semantic_order++;
    return item;
}
static void passive_clear(SBDesktop *d) {
    for (size_t i=0;i<d->passive_count;++i) { free(d->passive[i].text); free(d->passive[i].styles); }
    d->passive_count=0;
}
static void passive_span(SBDesktop *d,const char *id,const char *text,size_t length,accesskit_role role,struct nk_rect bounds) {
    if (!text || (!length && role!=ACCESSKIT_ROLE_CELL && role!=ACCESSKIT_ROLE_COLUMN_HEADER)) return;
    if (d->passive_count==d->passive_capacity) {
        size_t capacity=d->passive_capacity ? d->passive_capacity*2 : 32;
        SBPassiveText *next=realloc(d->passive,capacity*sizeof(*next)); if (!next) return;
        d->passive=next; d->passive_capacity=capacity;
    }
    char *copy=malloc(length+1); if (!copy) return;
    memcpy(copy,text,length); copy[length]=0;
    while (length && !sb_utf8_valid(copy,length)) copy[--length]=0;
    SBPassiveText *p=&d->passive[d->passive_count]; memset(p,0,sizeof(*p));
    if (id) snprintf(p->id,sizeof(p->id),"%s",id); else snprintf(p->id,sizeof(p->id),"caption:%zu",d->passive_count);
    p->text=copy; p->role=role; p->order=d->semantic_order++; p->group=d->focus_group;
    struct nk_rect clip=d->ui.ctx->current->layout->clip;
    float left=fmaxf(bounds.x,clip.x),top=fmaxf(bounds.y,clip.y);
    p->bounds=nk_rect(left,top,fmaxf(0,fminf(bounds.x+bounds.w,clip.x+clip.w)-left),fmaxf(0,fminf(bounds.y+bounds.h,clip.y+clip.h)-top));
    /* Nuklear's native window title sits outside its content clip. */
    if (id && !strcmp(id,"modal-title")) p->bounds=bounds;
    ++d->passive_count;
}
static void passive_add(SBDesktop *d,const char *id,const char *text,accesskit_role role,struct nk_rect bounds) {
    passive_span(d,id,text,text ? strlen(text) : 0,role,bounds);
}
static void passive_styles(SBDesktop *d,size_t before,const SBStyledText *styled,float font_size) {
    if (d->passive_count!=before+1 || !styled || strcmp(d->passive[before].text,styled->text)) return;
    SBPassiveText *p=&d->passive[before];
    if (sb_ui_styled_spans(styled,&p->styles,&p->style_count)) p->font_size=font_size;
}
static void document_span(SBDesktop *d,const char *text,size_t length,size_t offset,accesskit_role role,unsigned level,struct nk_rect bounds,unsigned slot,bool title) {
    char id[100]; snprintf(id,sizeof(id),"reader:block:%zu",offset);
    size_t before=d->passive_count; passive_span(d,id,text,length,role,bounds);
    if (before==d->passive_count) return;
    SBPassiveText *p=&d->passive[before]; strcpy(p->parent,"reader"); p->level=level;
    if (title) p->bounds=bounds;
    p->document_y=title ? 0 : fmaxf(0,bounds.y-d->ui.ctx->current->layout->clip.y+d->scrolling[slot].applied);
    if (d->reveal_document[0] && d->reveal_document_context!=accessible_context(d)) d->reveal_document[0]=0;
    if (!strcmp(d->reveal_document,id)) {
        d->scrolling[slot].destination=p->document_y; d->scrolling[slot].pending=d->scrolling[slot].elastic=0;
        d->scrolling[slot].active=true; d->reveal_document[0]=0;
    }
}
static void heading_step(SBDesktop *d,int direction) {
    uint64_t context=accessible_context(d); size_t current=d->passive_count,chosen=d->passive_count;
    unsigned slot=modal_reader(d) ? 2 : 0;
    if (d->heading_context==context)
        for (size_t i=0;i<d->passive_count;++i) if (!strcmp(d->heading_cursor,d->passive[i].id)) { current=i; break; }
    for (size_t i=0;i<d->passive_count;++i) {
        SBPassiveText *p=&d->passive[i];
        if (!p->parent[0] || p->role!=ACCESSKIT_ROLE_HEADING) continue;
        if (direction>0 && (current<d->passive_count ? i>current : p->document_y>d->scrolling[slot].destination+1)) { chosen=i; break; }
        if (direction<0 && (current<d->passive_count ? i<current : p->document_y<d->scrolling[slot].destination-1)) chosen=i;
    }
    if (chosen<d->passive_count) {
        snprintf(d->heading_cursor,sizeof(d->heading_cursor),"%s",d->passive[chosen].id);
        snprintf(d->reveal_document,sizeof(d->reveal_document),"%s",d->passive[chosen].id);
        d->heading_context=d->reveal_document_context=context;
    }
}
static void native_wrap(SBDesktop *d,const char *text) {
    passive_add(d,NULL,text,ACCESSKIT_ROLE_LABEL,nk_widget_bounds(d->ui.ctx));
    nk_label_wrap(d->ui.ctx,text);
}
static void native_label(SBDesktop *d,const char *text,nk_flags alignment) {
    passive_add(d,NULL,text,ACCESSKIT_ROLE_LABEL,nk_widget_bounds(d->ui.ctx));
    nk_label(d->ui.ctx,text,alignment);
}
static void target(SBDesktop *d, const char *id) {
    SBTarget *item = target_add(d,id,nk_widget_bounds(d->ui.ctx),SB_FOCUS_BUTTON,d->focus_group);
    if (!item) return;
    struct nk_panel *panel = d->ui.ctx->current->layout;
    struct nk_rect reveal = item->bounds;
    if (d->keyboard && d->focus_scroll_frames && !strcmp(d->focus,id)) {
        /* When a short group is clipped by its parent, reveal the group first. */
        while (panel && (panel->clip.h < item->bounds.h || !panel->offset_y)) { reveal = panel->bounds; panel = panel->parent; }
        if (panel) {
            float delta = reveal.y < panel->clip.y ? reveal.y-panel->clip.y :
                reveal.y+reveal.h > panel->clip.y+panel->clip.h ?
                reveal.y+reveal.h-panel->clip.y-panel->clip.h : 0;
            if (delta) *panel->offset_y = (nk_uint)fmaxf(0,(float)*panel->offset_y+delta);
        }
    }
}
static void target_label(SBDesktop *d, const char *label) {
    if (!d->target_count) return;
    char *out=d->targets[d->target_count-1].label;
    snprintf(out,SB_NAME_CAP,"%s",label);
    while (*out && !sb_utf8_valid(out,strlen(out))) out[strlen(out)-1]=0;
}
static void tooltip(SBDesktop *d, const char *text) {
    struct nk_context *ctx=d->ui.ctx;
    struct nk_style_item old=ctx->style.window.fixed_background;
    float rounding=ctx->style.window.rounding;
    ctx->style.window.fixed_background=nk_style_item_color(d->ui.dark ? nk_rgba(17,29,47,252) : nk_rgba(243,247,253,252));
    ctx->style.window.rounding=6;
    nk_tooltip(ctx,text);
    ctx->style.window.fixed_background=old; ctx->style.window.rounding=rounding;
}
static bool focused(SBDesktop *d, const char *id) { return d->keyboard && !strcmp(d->focus, id); }
static void ring(SBDesktop *d, const char *id) {
    if (focused(d, id) && d->target_count && !strcmp(d->targets[d->target_count-1].id,id)) {
        struct nk_rect bounds=d->targets[d->target_count-1].bounds;
        if (d->ui.contrast) {
            /* A separate inner ring remains distinct from the normal border. */
            float inset=4*d->ui.scale;
            bounds.x+=inset; bounds.y+=inset; bounds.w-=2*inset; bounds.h-=2*inset;
        }
        nk_stroke_rect(nk_window_get_canvas(d->ui.ctx),bounds,9,2,
            d->ui.contrast ? d->ui.ctx->style.text.color : nk_rgb(142,191,255));
    }
}
static bool activation(SBDesktop *d, const char *id) {
    if (strcmp(d->activate, id)) return false;
    d->activate[0] = 0; return true;
}
static bool button(SBDesktop *d, const char *id, const char *label) {
    struct nk_rect bounds = nk_widget_bounds(d->ui.ctx);
    target(d, id); target_label(d,!strcmp(id,"new-note") ? "Neue Notiz" : !strcmp(id,"new-project") ? "Neues Projekt" : !strcmp(id,"filter") ? "Wissensbereich wählen" : label);
    struct nk_rect clip = d->ui.ctx->current->layout->clip;
    if (bounds.y >= clip.y && bounds.y+bounds.h <= clip.y+clip.h) glass(d,bounds,9);
    SBIcon icon=sb_icon_for(id);
    if (icon==SB_ICON_EXPAND && d->expanded) icon=SB_ICON_COLLAPSE;
    bool only= !strcmp(id,"close-card") || !strcmp(id,"cancel") || !strcmp(id,"clear-search") ||
        !strcmp(id,"zoom-in") || !strcmp(id,"zoom-out") || !strncmp(id,"rotate-",7) || !strcmp(id,"camera-home") || !strcmp(id,"settings");
    only = only || (icon!=SB_ICON_NONE && bounds.w<70*d->ui.scale);
    bool clicked=nk_button_label(d->ui.ctx,"") != 0;
    struct nk_command_buffer *canvas=nk_window_get_canvas(d->ui.ctx);
    if (icon!=SB_ICON_NONE) {
        float size=fminf(20*d->ui.scale,bounds.h-12);
        sb_icon_draw(canvas,icon,nk_rect(only ? bounds.x+(bounds.w-size)/2 : bounds.x+10,bounds.y+(bounds.h-size)/2,size,size),d->ui.ctx->style.text.color);
    }
    if (!only) {
        char shown[SB_NAME_CAP];
        float inset=icon==SB_ICON_NONE ? 10 : 36*d->ui.scale;
        compact_label(d,label,shown,sizeof(shown),bounds.w-inset-12);
        struct nk_rect text=nk_rect(bounds.x+inset,bounds.y+(bounds.h-d->ui.normal->handle.height)/2,bounds.w-inset-10,d->ui.normal->handle.height);
        nk_draw_text(canvas,text,shown,(int)strlen(shown),&d->ui.normal->handle,nk_rgba(0,0,0,0),d->ui.ctx->style.text.color);
    }
    if (nk_input_is_mouse_hovering_rect(&d->ui.ctx->input,bounds)) tooltip(d,label);
    ring(d, id);
    return clicked || activation(d, id);
}
static bool selectable(SBDesktop *d, const char *id, const char *label, nk_bool *selected) {
    target(d, id); target_label(d,label);
    bool clicked = nk_selectable_label(d->ui.ctx, label, NK_TEXT_LEFT, selected) != 0;
    ring(d, id);
    return clicked || activation(d, id);
}
static void text_target(SBDesktop *d, const char *id) {
    target(d, id);
    if (d->target_count && !strcmp(d->targets[d->target_count-1].id,id)) d->targets[d->target_count - 1].kind = SB_FOCUS_TEXT;
    if (focused(d, id) && !(d->ui.ctx->current->layout->flags & NK_WINDOW_NO_INPUT)) {
        nk_window_set_focus(d->ui.ctx, d->ui.ctx->current->name_string);
        /* Closing a higher window can leave Nuklear's lower window read-only. */
        d->ui.ctx->current->flags &= ~(nk_flags)NK_WINDOW_ROM;
        d->ui.ctx->current->layout->flags &= ~(nk_flags)NK_WINDOW_ROM;
        /* Widget sequence numbers can change when another field scrolls offscreen. */
        nk_edit_focus(d->ui.ctx, NK_EDIT_ALWAYS_INSERT_MODE); d->focus_changed = false;
    }
}
static void command(SBDesktop *d, SBCommand cmd) { if (d->command == SB_CMD_NONE) d->command = cmd; }
static void result(SBDesktop *d, SBStatus status, const char *success) {
    d->message = status;
    if (status.code != SB_OK) d->card = true;
    if (status.code == SB_OK && success) snprintf(d->message.message, sizeof(d->message.message), "%s", success);
}
static void backup_poll(SBDesktop *d) {
    if (!d->backup) return;
    sb_backup_job_snapshot(d->backup,&d->backup_state);
    SBBackupJobState *state=&d->backup_state;
    if (!state->done) return;
    sb_backup_job_free(d->backup); d->backup=NULL;
    d->message=state->status;
    d->backup_error_copied=d->backup_clipboard_failed=false;
    if (state->status.code!=SB_OK && state->status.code!=SB_CANCELLED) d->backup_feedback_reset=true;
    if (state->status.code==SB_CANCELLED || (state->kind==SB_JOB_INSPECT && state->cancel_requested)) { result(d,sb_ok(),"Abgebrochen."); d->form=SB_FORM_NONE; }
    else if (state->status.code==SB_OK && state->kind==SB_JOB_INSPECT) {
        d->restore_info=state->info; d->restore_checked=true;
        snprintf(d->checked_backup,sizeof(d->checked_backup),"%s",d->backup_path);
        strcpy(d->restore_id,state->info.id);
        char path[SB_PATH_CAP]; sb_path_join(path,sizeof(path),d->model.workspace,d->restore_id);
        if (sb_fs_kind(path)!=0) {
            size_t n=strlen(state->info.id); if (n>58) n=58;
            while (n && state->info.id[n-1]=='-') --n;
            snprintf(d->restore_id,sizeof(d->restore_id),"%.*s-kopie",(int)n,state->info.id);
        }
        snprintf(d->focus,sizeof(d->focus),"restore-id"); d->keyboard=true; d->focus_changed=true; d->focus_scroll_frames=3; d->form_focus=2;
        result(d,sb_ok(),"Sicherung geprüft.");
    } else if (state->status.code==SB_OK) {
        if (state->kind==SB_JOB_RESTORE) {
            SBProjects projects={0}; SBStatus listed=sb_projects_scan(d->model.workspace,&projects);
            if (listed.code==SB_OK) {
                sb_projects_free(&d->model.projects); d->model.projects=projects;
                if (!d->model.has_project) listed=sb_app_request(&d->model,SB_ACT_PROJECT,state->project.id);
            }
            if (listed.code!=SB_OK) d->message=sb_error(listed.code,"Projekt wiederhergestellt. Projektliste konnte nicht aktualisiert werden: %s",listed.message);
            else result(d,sb_ok(),state->cancel_requested ? "Projekt war vor dem Abbruch bereits wiederhergestellt." : "Projekt wiederhergestellt. Es ist unter Projekte verfügbar.");
        } else result(d,sb_ok(),state->cancel_requested ? "Sicherung war vor dem Abbruch bereits abgeschlossen." : "Sicherung gespeichert.");
        d->form=SB_FORM_NONE;
    } else if (state->kind==SB_JOB_RESTORE && (state->status.code==SB_CONFLICT || state->status.code==SB_INVALID)) d->restore_checked=false;
    if (d->quit_after_backup) {
        d->quit_after_backup=false; SBStatus status=sb_app_request(&d->model,SB_ACT_QUIT,NULL);
        if (status.code!=SB_OK) d->message=status;
    }
}
static void field(SBDesktop *d, const char *tag, const char *label, char *text, int capacity, int focus) {
    struct nk_context *ctx = d->ui.ctx;
    int length = (int)strlen(text);
    nk_layout_row_dynamic(ctx, 22 * d->ui.scale, 1);
    nk_label(ctx, label, NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 36 * d->ui.scale, 1);
    text_target(d, tag); target_label(d,label);
    if (d->form_focus == focus) { nk_edit_focus(ctx, NK_EDIT_ALWAYS_INSERT_MODE); d->form_focus = 0; }
    nk_flags state = nk_edit_string(ctx, NK_EDIT_FIELD, text, &length, capacity, nk_filter_default);
    if (state & NK_EDIT_ACTIVE) d->active_form_field = focus;
    text[length] = 0;
    ring(d, tag);
}
static void request(SBDesktop *d, SBActionKind kind, const char *value) {
    if (d->navigation.kind != SB_ACT_NONE) return;
    if (value && strlen(value) >= sizeof(d->navigation.value)) {
        d->message = sb_error(SB_LIMIT, "Zielpfad zu lang."); return;
    }
    d->navigation.kind = kind;
    if (kind == SB_ACT_NOTE && value && strcmp(value,d->model.path)) d->follow_star=true;
    if (value) strcpy(d->navigation.value, value);
    else d->navigation.value[0] = 0;
}
SBStatus sb_desktop_init(SBDesktop *d, const char *workspace, const char *font, bool testing) {
    SBStatus status;
    memset(d, 0, sizeof(*d));
    d->flight=1;
    d->sidebar = true; d->test = testing; d->card = true; d->zoom = d->view_zoom = 1; d->seconds = 1.0f/60; d->graph_dirty = true;
    strcpy(d->focus, "project-picker"); strcpy(d->section, "all");
    status = sb_ui_init(&d->ui, font, 1336, 840, testing);
    if (status.code != SB_OK) return status;
    d->dialogs=sb_dialogs_new();
    d->accessibility=sb_accessibility_new(d->ui.window);
    d->style_monitor=sb_system_style_new(!testing);
    d->requested_style.dark=true; style_update(d,true);
    status = sb_app_init(&d->model, workspace);
    if (status.code != SB_OK) {
        sb_fs_absolute(workspace, d->model.workspace, sizeof(d->model.workspace));
        d->message = status;
    }
    d->generation = d->model.generation;
    return sb_ok();
}
SBStatus sb_desktop_preferences(SBDesktop *d, const char *path, bool explicit_workspace) {
    SBSettings s; SBRevision revision;
    if (strlen(path)>=sizeof(d->settings_path)) return sb_error(SB_LIMIT,"Einstellungspfad ist zu lang.");
    SBStatus status=sb_settings_load(path,&s,&revision);
    if (status.code!=SB_OK) { d->message=status; return status; }
    strcpy(d->settings_path,path); d->settings_revision=revision; d->settings_enabled=true;
    sb_desktop_set_style(d,(SBStyleChoice){.dark=s.dark,.solid=s.solid,.motion=s.reduced_motion,.contrast=s.contrast,.follow_theme=s.follow_theme});
    status=sb_ui_fonts(&d->ui,s.font_percent/100.0f);
    if (status.code!=SB_OK) { d->message=status; return status; }
    if (!SDL_SetWindowSize(d->ui.window,(int)s.width,(int)s.height) || !SDL_SyncWindow(d->ui.window)) {
        d->settings_enabled=false;
        d->message=sb_error(SB_IO,"Fenstergröße konnte nicht wiederhergestellt werden: %s",SDL_GetError());
        return d->message;
    }
    if (!explicit_workspace && s.workspace[0]) {
        if (sb_fs_kind(s.workspace)!=2) {
            d->settings_enabled=false;
            d->message=sb_error(SB_NOT_FOUND,"Der letzte Arbeitsordner ist nicht erreichbar. Wähle einen Arbeitsordner.");
            return d->message;
        }
        status=sb_app_request(&d->model,SB_ACT_WORKSPACE,s.workspace);
        if (status.code!=SB_OK) { d->settings_enabled=false; d->message=status; return status; }
        if (s.project[0]) {
            status=sb_app_request(&d->model,SB_ACT_PROJECT,s.project);
            if (status.code==SB_OK && s.note[0]) status=sb_app_request(&d->model,SB_ACT_NOTE,s.note);
            if (status.code!=SB_OK) d->message=sb_error(SB_NOT_FOUND,"Das zuletzt geöffnete Dokument ist nicht mehr verfügbar.");
        }
    }
    return sb_ok();
}
SBStatus sb_desktop_store_preferences(SBDesktop *d) {
    if (!d->settings_enabled) return sb_ok();
    SBSettings s; sb_settings_defaults(&s);
    snprintf(s.workspace,sizeof(s.workspace),"%s",d->model.workspace);
    if (d->model.has_project) {
        snprintf(s.project,sizeof(s.project),"%s",d->model.project.id);
        snprintf(s.note,sizeof(s.note),"%s",d->model.path);
    }
    int width,height;
    if (!SDL_SyncWindow(d->ui.window) || !SDL_GetWindowSize(d->ui.window,&width,&height)) {
        d->message=sb_error(SB_IO,"Fenstergröße konnte nicht gespeichert werden: %s",SDL_GetError());
        return d->message;
    }
    s.width=(unsigned)fmaxf(780,fminf(8192,(float)width)); s.height=(unsigned)fmaxf(520,fminf(8192,(float)height));
    s.font_percent=(unsigned)roundf(d->ui.scale*100); s.dark=d->requested_style.dark; s.solid=d->requested_style.solid; s.reduced_motion=d->requested_style.motion;
    s.follow_theme=d->requested_style.follow_theme; s.contrast=d->requested_style.contrast;
    SBStatus status=sb_settings_save(d->settings_path,&s,d->settings_revision,&d->settings_revision);
    if (status.code!=SB_OK) d->message=status;
    return status;
}
void sb_desktop_free(SBDesktop *d) {
    sb_system_style_free(d->style_monitor);
    sb_accessibility_free(d->accessibility);
    sb_backup_job_free(d->backup);
    sb_dialogs_free(d->dialogs);
    if (d->text_edit_ready) nk_textedit_free(&d->text_edit);
    free(d->targets); passive_clear(d); free(d->passive);
    sb_graph_free(&d->graph);
    sb_notes_free(&d->hits); free(d->context); free(d->notice); sb_app_free(&d->model);
    sb_ui_shutdown(&d->ui); memset(d, 0, sizeof(*d));
}
static void search_refresh(SBDesktop *d) {
    if (!strcmp(d->search, d->searched)) return;
    SBNotes hits = {0}; d->page = 0;
    if (d->model.has_project && d->search[0]) {
        SBStatus status = sb_search(&d->model.project, d->search, &hits);
        if (status.code != SB_OK) { d->message = status; return; }
    }
    sb_notes_free(&d->hits); d->hits = hits;
    strcpy(d->searched, d->search);
}
static void editor_reset(SBDesktop *d) {
    if (d->text_edit_ready) nk_textedit_free(&d->text_edit);
    d->text_edit_ready = false;
    if (!d->model.editor) return;
    nk_textedit_init_fixed(&d->text_edit,d->model.editor,SB_TEXT_LIMIT);
    d->text_edit.string.buffer.allocated = strlen(d->model.editor);
    d->text_edit.string.len = 0;
    for (const unsigned char *p = (const unsigned char *)d->model.editor; *p; ++p)
        if ((*p & 0xc0) != 0x80) ++d->text_edit.string.len;
    d->text_edit.mode = NK_TEXT_EDIT_MODE_INSERT;
    d->text_edit_ready = true;
}
static void synchronize(SBDesktop *d) {
    if (d->generation != d->model.generation) {
        d->generation = d->model.generation;
        d->editing = d->next_edit; d->next_edit = false;
        d->focus_editor = d->editing;
        d->reset_reader = true; d->card = true; d->graph_dirty = true; d->page = 0;
        memset(d->scrolling,0,sizeof(d->scrolling));
        sb_ui_reset_editor(&d->ui); editor_reset(d);
        d->search[0] = 0; d->searched[0] = 0; sb_notes_free(&d->hits);
        if (strcmp(d->section,"all")) {
            bool present=false;
            for (size_t i=0;i<d->model.notes.count;++i)
                if (!strcmp(d->model.notes.items[i].path,d->model.path) && !strcmp(d->model.notes.items[i].section,d->section)) present=true;
            if (!present) strcpy(d->section,"all");
        }
    }
    if (!d->text_edit_ready && d->model.editor) editor_reset(d);
}
void sb_desktop_apply(SBDesktop *d) {
    SBCommand cmd = d->command;
    SBStatus status = sb_ok();
    d->command = SB_CMD_NONE;
    if (d->backup) { if (cmd==SB_CMD_CANCEL) sb_backup_job_cancel(d->backup); return; }
    if (cmd==SB_CMD_BACKUP || cmd==SB_CMD_RESTORE) {
        d->backup_error_copied=d->backup_clipboard_failed=false;
        ++d->dialog_serial; d->message=sb_ok(); d->restore_checked=false; d->checked_backup[0]=0;
        d->form=cmd==SB_CMD_BACKUP ? SB_FORM_BACKUP : SB_FORM_RESTORE; d->form_focus=1;
        d->restore_id[0]=0; d->backup_path[0]=0;
        if (cmd==SB_CMD_BACKUP && d->model.has_project) {
            char name[100],date[32]; time_t now=time(NULL); struct tm *tm=localtime(&now);
            if (tm) strftime(date,sizeof(date),"%Y%m%d-%H%M%S",tm); else strcpy(date,"sicherung");
            snprintf(name,sizeof(name),"%s-%s.sbbackup",d->model.project.id,date);
            d->message=sb_path_join(d->backup_path,sizeof(d->backup_path),d->model.workspace,name);
        }
        return;
    }
    if (cmd==SB_CMD_INSPECT || (cmd==SB_CMD_SUBMIT && (d->form==SB_FORM_BACKUP || d->form==SB_FORM_RESTORE))) {
        SBBackupJobKind kind=d->form==SB_FORM_BACKUP ? SB_JOB_BACKUP : cmd==SB_CMD_INSPECT || !d->restore_checked ? SB_JOB_INSPECT : SB_JOB_RESTORE;
        if (!d->backup_path[0]) status=sb_error(SB_INVALID,"Wähle eine Sicherungsdatei.");
        else if (kind==SB_JOB_BACKUP && !d->model.has_project) status=sb_error(SB_INVALID,"Wähle zuerst ein Projekt.");
        else if (kind==SB_JOB_BACKUP && sb_app_dirty(&d->model)) {
            status=sb_app_save(&d->model); if (status.code==SB_OK) { d->graph_dirty=true; d->searched[0]=0; }
        }
        else if (kind==SB_JOB_RESTORE && !sb_id_valid(d->restore_id)) status=sb_error(SB_INVALID,"Wähle einen gültigen Projektordnernamen.");
        if (status.code==SB_OK) {
            ++d->dialog_serial;
            d->backup=sb_backup_job_start(kind,&d->model.project,d->backup_path,d->model.workspace,d->restore_id,kind==SB_JOB_RESTORE ? d->restore_info.digest : NULL);
            if (!d->backup) status=sb_error(SB_IO,"Sicherungsvorgang konnte nicht gestartet werden: %s",SDL_GetError());
        }
        d->message=status; return;
    }
    if (cmd == SB_CMD_NEW_PROJECT || cmd == SB_CMD_NEW_NOTE) {
        d->form = cmd == SB_CMD_NEW_PROJECT ? SB_FORM_PROJECT : SB_FORM_NOTE;
        d->name[0] = 0; d->id[0] = 0; d->repository[0] = 0; d->id_manual = false;
        d->note_section = 0; d->form_focus = 1; d->active_form_field = 1;
    } else if (cmd == SB_CMD_WORKSPACE) {
        ++d->dialog_serial; d->form = SB_FORM_WORKSPACE;
        strcpy(d->folder, d->model.workspace); d->form_focus = 1;
    } else if (cmd == SB_CMD_SUBMIT) {
        if (d->form == SB_FORM_PROJECT) {
            status = sb_app_new_project(&d->model, d->id, d->name, d->repository);
            if (status.code == SB_OK) strcpy(d->section, "all");
            result(d, status, "Projekt angelegt.");
        } else if (d->form == SB_FORM_NOTE) {
            status = sb_app_new_note(&d->model, new_sections[d->note_section], d->id, d->name);
            if (status.code == SB_OK) { d->next_edit = true; strcpy(d->section, new_sections[d->note_section]); }
            result(d, status, "Notiz angelegt.");
        } else if (d->form == SB_FORM_WORKSPACE) {
            status = sb_app_request(&d->model, SB_ACT_WORKSPACE, d->folder);
            if (status.code == SB_OK && d->settings_path[0]) d->settings_enabled=true;
            result(d, status, "Arbeitsordner geöffnet.");
        }
        if (status.code == SB_OK) d->form = SB_FORM_NONE;
    } else if (cmd == SB_CMD_CANCEL) { ++d->dialog_serial; d->form = SB_FORM_NONE; }
    else if (cmd == SB_CMD_SAVE) {
        status = sb_app_save(&d->model); result(d, status, "Gespeichert.");
        if (status.code == SB_OK) d->graph_dirty = true;
        d->searched[0] = 0;
    } else if (cmd == SB_CMD_COPY) {
        status = sb_app_save_copy(&d->model); result(d, status, "Eigene Fassung als neue Wissensnotiz gespeichert.");
    } else if (cmd == SB_CMD_CONTEXT) {
        char *text = NULL;
        status = sb_context_build(&d->model.project, &text);
        if (status.code == SB_OK) {
            free(d->context); d->context = text; d->form = SB_FORM_CONTEXT;
            result(d, status, sb_app_dirty(&d->model) ? "Kontext verwendet gespeicherte Dateien. Aktuelle Änderungen sind noch ungespeichert." : "Kontext aus den gespeicherten Kerninformationen.");
        } else result(d, status, NULL);
    } else if (cmd == SB_CMD_GUARD_SAVE || cmd == SB_CMD_GUARD_DISCARD || cmd == SB_CMD_GUARD_CANCEL) {
        status = sb_app_decide(&d->model, cmd == SB_CMD_GUARD_SAVE ? SB_SAVE_CHANGES :
                              cmd == SB_CMD_GUARD_DISCARD ? SB_DISCARD_CHANGES : SB_KEEP_EDITING);
        if (cmd == SB_CMD_GUARD_CANCEL) { d->next_edit = false; d->follow_star=false; }
        result(d, status, NULL);
    } else if (cmd == SB_CMD_ARCHIVE || cmd == SB_CMD_RELOAD) {
        status = sb_app_request(&d->model, cmd == SB_CMD_ARCHIVE ? SB_ACT_ARCHIVE : SB_ACT_RELOAD, NULL);
        result(d, status, cmd == SB_CMD_ARCHIVE ? "Dokument archiviert." : "Dokument neu geladen.");
    } else if (cmd == SB_CMD_THEME) { SBStyleChoice style=d->requested_style; style.dark=!d->ui.dark; style.follow_theme=false; sb_desktop_set_style(d,style); }
    else if (cmd == SB_CMD_SCALE) result(d, sb_ui_fonts(&d->ui, d->next_scale), NULL);
    else if (cmd == SB_CMD_SOURCE) {
        if (!d->model.source) snprintf(d->source_focus,sizeof(d->source_focus),"%s",d->focus);
        result(d, sb_app_source(&d->model, d->command_value), NULL);
        if (d->message.code == SB_OK) d->reset_reader = true;
    }
    if (d->navigation.kind != SB_ACT_NONE) {
        SBAction action = d->navigation;
        memset(&d->navigation, 0, sizeof(d->navigation));
        result(d, sb_app_request(&d->model, action.kind, action.value), NULL);
        if (action.kind == SB_ACT_PROJECT && (d->form == SB_FORM_SETTINGS || d->form == SB_FORM_PROJECTS) && d->message.code == SB_OK)
            d->form = SB_FORM_NONE;
    }
    synchronize(d);
}
static SBTarget *focus_target(SBDesktop *d) {
    for (size_t i = 0; i < d->target_count; ++i) if (!strcmp(d->focus, d->targets[i].id)) return &d->targets[i];
    return NULL;
}
static void focus_set(SBDesktop *d, const char *id) {
    snprintf(d->focus, sizeof(d->focus), "%s", id);
    d->focus_changed = true; d->keyboard = true; d->focus_scroll_frames = 3;
    for (unsigned i=0;i<4;++i) { d->scrolling[i].pending=0; d->scrolling[i].elastic=0; d->scrolling[i].ready=false; d->scrolling[i].active=false; }
    d->ui.ctx->text_edit.active = 0;
    for (struct nk_window *w = d->ui.ctx->begin; w; w = w->next) w->edit.active = 0;
}
static char *accessible_field(SBDesktop *d,const char *id,size_t *capacity) {
#define FIELD(tag,member) if (!strcmp(id,tag)) { *capacity=sizeof(d->member); return d->member; }
    FIELD("search",search) FIELD("form-name",name) FIELD("form-id",id)
    FIELD("form-repo",repository) FIELD("form-folder",folder)
    FIELD("backup-path",backup_path) FIELD("restore-id",restore_id)
#undef FIELD
    *capacity=0; return NULL;
}
static void accessible_actions(SBDesktop *d) {
    SBAccessibleAction action;
    while (sb_accessibility_next_action(d->accessibility,&action)) {
        if (!sb_accessibility_current(d->accessibility,&action,accessible_context(d))) { sb_accessibility_action_free(&action); continue; }
        SBTarget *target=NULL;
        for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->targets[i].id,action.id)) { target=&d->targets[i]; break; }
        if ((!strncmp(action.id,"reader:block:",13) || !strncmp(action.id,"reader:table:",13)) && action.action==ACCESSKIT_ACTION_SCROLL_INTO_VIEW) {
            snprintf(d->reveal_document,sizeof(d->reveal_document),"%s",action.id); d->reveal_document_context=accessible_context(d);
        } else if (!strncmp(action.id,"star:",5) && d->form==SB_FORM_NONE && !d->model.guard) {
            char *end=NULL; unsigned long index=strtoul(action.id+5,&end,10);
            if (end && !*end && index<d->model.notes.count && (action.action==ACCESSKIT_ACTION_CLICK || action.action==ACCESSKIT_ACTION_FOCUS)) {
                d->card=true; d->star=index; d->follow_star=true; focus_set(d,"galaxy"); request(d,SB_ACT_NOTE,d->model.notes.items[index].path);
            }
        } else if (target) {
            if (action.action==ACCESSKIT_ACTION_FOCUS || action.action==ACCESSKIT_ACTION_SCROLL_INTO_VIEW) focus_set(d,action.id);
            else if (action.action==ACCESSKIT_ACTION_CLICK && target->kind==SB_FOCUS_BUTTON) { focus_set(d,action.id); snprintf(d->activate,sizeof(d->activate),"%s",action.id); }
            else if (action.action==ACCESSKIT_ACTION_SCROLL_UP || action.action==ACCESSKIT_ACTION_SCROLL_DOWN) {
                unsigned slot=modal_reader(d) ? 2 : d->form!=SB_FORM_NONE ? 3 : !strcmp(action.id,"editor") ? 1 : 0;
                d->scrolling[slot].pending+=action.action==ACCESSKIT_ACTION_SCROLL_DOWN ? 220 : -220;
            } else if (!strcmp(action.id,"editor") && d->editing && d->text_edit_ready && !d->model.source && !d->model.guard) {
                struct nk_text_edit *edit=&d->text_edit;
                size_t total=(size_t)nk_utf_len(d->model.editor,(int)strlen(d->model.editor));
                if (action.action==ACCESSKIT_ACTION_SET_TEXT_SELECTION && action.anchor<=total && action.caret<=total) {
                    edit->select_start=(int)action.anchor; edit->select_end=edit->cursor=(int)action.caret; sb_ui_grapheme_clamp(edit); focus_set(d,"editor");
                } else if ((action.action==ACCESSKIT_ACTION_SET_VALUE || action.action==ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT) && action.value && sb_utf8_valid(action.value,strlen(action.value))) {
                    size_t n=strlen(action.value);
                    if (n<SB_TEXT_LIMIT) {
                        int start=edit->select_start,end=edit->select_end;
                        if (action.action==ACCESSKIT_ACTION_SET_VALUE) nk_textedit_select_all(edit);
                        bool success=n ? nk_textedit_paste(edit,action.value,(int)n)!=0 : nk_textedit_cut(edit)!=0;
                        if (success) { d->model.editor[edit->string.buffer.allocated]=0; focus_set(d,"editor"); }
                        else { edit->select_start=start; edit->select_end=end; d->message=sb_error(SB_LIMIT,"Text konnte nicht eingefügt werden."); }
                    } else d->message=sb_error(SB_LIMIT,"Text überschreitet die Editorgrenze.");
                }
            } else if (target->kind==SB_FOCUS_TEXT && action.action==ACCESSKIT_ACTION_SET_VALUE && action.value) {
                size_t capacity=0; char *field=accessible_field(d,action.id,&capacity); size_t n=strlen(action.value);
                if (field && n<capacity && sb_utf8_valid(action.value,n) && !strchr(action.value,'\n') && !strchr(action.value,'\r')) {
                    strcpy(field,action.value); if (!strcmp(action.id,"form-id")) d->id_manual=true;
                    if (!strcmp(action.id,"backup-path")) d->restore_checked=false;
                    focus_set(d,action.id);
                } else d->message=sb_error(SB_INVALID,"Eingabe ist ungültig oder zu lang.");
            }
        }
        sb_accessibility_action_free(&action);
    }
}
static int accessible_order(const void *left,const void *right) {
    const SBAccessibleItem *a=left,*b=right;
    return a->order<b->order ? -1 : a->order>b->order;
}
static void accessible_publish(SBDesktop *d) {
    if (!d->accessibility) return;
    bool modal=d->form!=SB_FORM_NONE || d->model.guard;
    size_t stars=modal ? 0 : d->model.notes.count;
    size_t controls=d->target_count+d->passive_count;
    size_t count=controls+stars;
    SBAccessibleItem *items=calloc(count,sizeof(*items)); char (*ids)[100]=stars ? calloc(stars,sizeof(*ids)) : NULL;
    if ((count && !items) || (stars && !ids) ) { free(items); free(ids); return; }
    for (size_t i=0;i<d->target_count;++i) {
        SBTarget *t=&d->targets[i]; SBAccessibleItem *v=&items[i]; v->id=t->id; v->label=t->label; v->bounds=t->bounds;
        v->role=t->kind==SB_FOCUS_TEXT ? !strcmp(t->id,"editor") ? ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT : !strcmp(t->id,"search") ? ACCESSKIT_ROLE_SEARCH_INPUT : ACCESSKIT_ROLE_TEXT_INPUT :
            t->kind==SB_FOCUS_READER ? ACCESSKIT_ROLE_DOCUMENT : t->kind==SB_FOCUS_MAP ? ACCESSKIT_ROLE_GROUP : !strncmp(t->id,"link:",5) ? ACCESSKIT_ROLE_LINK : ACCESSKIT_ROLE_BUTTON;
        if (t->kind==SB_FOCUS_READER && d->form==SB_FORM_NONE && d->reader_title_bounds.w>0) {
            struct nk_rect r=d->reader_title_bounds;
            float bottom=fmaxf(v->bounds.y+v->bounds.h,r.y+r.h);
            v->bounds.y=fminf(v->bounds.y,r.y); v->bounds.h=bottom-v->bounds.y;
        }
        if (t->kind==SB_FOCUS_TEXT) {
            size_t capacity=0; v->value=!strcmp(t->id,"editor") ? d->model.editor : accessible_field(d,t->id,&capacity); v->editable=true;
            if (!strcmp(t->id,"editor") && d->text_edit_ready) { v->anchor=(size_t)d->text_edit.select_start; v->caret=(size_t)d->text_edit.select_end; }
        } else if (t->kind==SB_FOCUS_READER) v->value=d->form==SB_FORM_NOTICE_TEXT ? d->notice : d->form==SB_FORM_CONTEXT ? d->context : d->model.source ? d->model.source : d->model.editor;
        if (!strcmp(t->id,"galaxy")) v->label="Dokumente in der Sternkarte";
        v->order=((uint64_t)t->group<<32)|t->order;
        if (t->parent[0]) v->parent=t->parent;
        else if (!strncmp(t->id,"link:",5)) v->parent="reader";
    }
    for (size_t i=0;i<d->passive_count;++i) {
        SBPassiveText *p=&d->passive[i]; SBAccessibleItem *v=&items[d->target_count+i];
        v->id=p->id; v->label=p->role==ACCESSKIT_ROLE_LIST || p->role==ACCESSKIT_ROLE_LIST_ITEM || p->role==ACCESSKIT_ROLE_BLOCKQUOTE || p->role==ACCESSKIT_ROLE_LIST_MARKER || p->role==ACCESSKIT_ROLE_TABLE || p->role==ACCESSKIT_ROLE_ROW || p->role==ACCESSKIT_ROLE_CELL || p->role==ACCESSKIT_ROLE_COLUMN_HEADER || p->role==ACCESSKIT_ROLE_SPLITTER || !p->parent[0] || p->role==ACCESSKIT_ROLE_HEADING ? p->text : ""; v->value=p->role==ACCESSKIT_ROLE_LIST || p->role==ACCESSKIT_ROLE_LIST_ITEM || p->role==ACCESSKIT_ROLE_BLOCKQUOTE || p->role==ACCESSKIT_ROLE_TABLE || p->role==ACCESSKIT_ROLE_ROW || p->role==ACCESSKIT_ROLE_SPLITTER ? NULL : p->text; v->bounds=p->bounds; v->role=p->role;
        v->row=p->row; v->column=p->column; v->rows=p->rows; v->columns=p->columns;
        v->styles=p->styles; v->style_count=p->style_count; v->font_size=p->font_size;
        v->parent=p->parent; v->level=p->level; v->order=((uint64_t)p->group<<32)|p->order;
    }
    qsort(items,controls,sizeof(*items),accessible_order);
    for (size_t i=0;i<stars;++i) {
        snprintf(ids[i],100,"star:%zu",i); SBAccessibleItem *v=&items[controls+i];
        v->id=ids[i]; v->label=d->model.notes.items[i].title; v->role=ACCESSKIT_ROLE_LIST_BOX_OPTION;
        v->selected=!strcmp(d->model.path,d->model.notes.items[i].path);
        if (i<d->ui.space.count && d->ui.space.points[i].visible) {
            SBPoint p=d->ui.space.points[i]; v->bounds=nk_rect(p.x-10,p.y-10,20,20);
        }
    }
    char title[SB_NAME_CAP]; snprintf(title,sizeof(title),"%s",d->model.has_project ? d->model.project.name : "SecondBrain");
    const char *status=d->message.message;
    char progress[150];
    if (d->backup) { snprintf(progress,sizeof(progress),"Sicherung: %zu von %zu Einträgen, %llu Bytes",d->backup_state.entries,d->backup_state.total,(unsigned long long)d->backup_state.bytes); status=progress; }
    char native_focus[100]; snprintf(native_focus,sizeof(native_focus),"%s",d->focus);
    if (!strcmp(d->focus,"galaxy") && d->star<stars) snprintf(native_focus,sizeof(native_focus),"star:%zu",d->star);
    sb_accessibility_update(d->accessibility,title,native_focus,items,count,status,modal,d->semantic_context);
    free(items); free(ids);
}
static void focus_step(SBDesktop *d, int direction) {
    if (!d->target_count) return;
    size_t index = direction > 0 ? d->target_count - 1 : 0;
    for (size_t i = 0; i < d->target_count; ++i) if (!strcmp(d->focus, d->targets[i].id)) { index = i; break; }
    index = direction > 0 ? (index + 1) % d->target_count : (index + d->target_count - 1) % d->target_count;
    focus_set(d, d->targets[index].id);
}
static bool inside(float x, float y, struct nk_rect r) {
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}
static bool map_input(SBDesktop *d, float x, float y) {
    if (!d->model.has_project) return false;
    if (d->form != SB_FORM_NONE || d->model.guard || !inside(x, y, d->map_bounds)) return false;
    for (unsigned i = 0; i < d->ui.space.glass_count; ++i) {
        SDL_FRect r = d->ui.space.glass[i].rect;
        if (inside(x, y, nk_rect(r.x, r.y, r.w, r.h))) return false;
    }
    return true;
}
static void camera_reset(SBDesktop *d) {
    d->yaw=d->pitch=d->pan_x=d->pan_y=0; d->zoom=1;
    d->flight_from[0]=d->focus_x; d->flight_from[1]=d->focus_y; d->flight_from[2]=d->focus_z;
    memset(d->flight_to,0,sizeof(d->flight_to)); d->flight=0;
}
static void star_step(SBDesktop *d, SDL_Keycode key) {
    SBSpace *s = &d->ui.space;
    if (!s->count) return;
    if (d->star >= s->count || !s->points[d->star].visible) {
        for (size_t i = 0; i < s->count; ++i) if (s->points[i].visible) { d->star = i; return; }
        return;
    }
    SBPoint current = s->points[d->star];
    float best = 1e20f; size_t found = d->star;
    for (size_t i = 0; i < s->count; ++i) if (i != d->star && s->points[i].visible) {
        float dx = s->points[i].x - current.x, dy = s->points[i].y - current.y;
        float along = key == SDLK_LEFT ? -dx : key == SDLK_RIGHT ? dx : key == SDLK_UP ? -dy : dy;
        float across = key == SDLK_LEFT || key == SDLK_RIGHT ? fabsf(dy) : fabsf(dx);
        float score = along + across * 2;
        if (along > 1 && score < best) { best = score; found = i; }
    }
    d->star = found;
}
void sb_desktop_event(SBDesktop *d, const SDL_Event *event) {
    if (d->dialogs && event->type==sb_dialogs_event(d->dialogs)) {
        SBDialogReply *reply=event->user.data1;
        bool folder=d->form==SB_FORM_WORKSPACE && reply && reply->kind==SB_DIALOG_FOLDER;
        bool backup=reply && ((d->form==SB_FORM_BACKUP && reply->kind==SB_DIALOG_SAVE_BACKUP) || (d->form==SB_FORM_RESTORE && reply->kind==SB_DIALOG_OPEN_BACKUP));
        if (reply && reply->serial==d->dialog_serial && !d->backup && (folder || backup)) {
            if (reply->error) d->message=sb_error(SB_IO,"%s",reply->value[0] ? reply->value : folder ? "Ordnerauswahl ist nicht verfügbar." : "Dateiauswahl ist nicht verfügbar.");
            else if (reply->value[0]) {
                if (folder) snprintf(d->folder,sizeof(d->folder),"%s",reply->value);
                else { snprintf(d->backup_path,sizeof(d->backup_path),"%s",reply->value); d->restore_checked=false; }
                d->message=sb_ok(); focus_set(d,folder ? "form-folder" : "backup-path");
                if (backup && d->form==SB_FORM_RESTORE) command(d,SB_CMD_INSPECT);
            }
        }
        SDL_free(reply); return;
    }
    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        if (d->backup) { sb_backup_job_cancel(d->backup); d->quit_after_backup=true; return; }
        request(d, SB_ACT_QUIT, NULL); return;
    }
    if (event->type==SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button==SDL_BUTTON_LEFT) {
        for (unsigned i=0;i<4;++i) {
            SBScroll *s=&d->scrolling[i];
            if (!d->model.guard && (d->form==SB_FORM_NONE ? i<2 : modal_reader(d) ? i==2 : i==3) && s->used && s->maximum>0 && inside(event->button.x,event->button.y,s->track)) {
                d->keyboard=false;
                if (inside(event->button.x,event->button.y,s->thumb)) { s->dragging=true; s->grab=event->button.y-s->thumb.y; }
                else s->pending+=(event->button.y<s->thumb.y ? -1 : 1)*s->height*0.9f;
                return;
            }
        }
    }
    if (event->type==SDL_EVENT_MOUSE_MOTION) {
        for (unsigned i=0;i<4;++i) if (d->scrolling[i].dragging) {
            SBScroll *s=&d->scrolling[i];
            float range=fmaxf(1,s->track.h-s->thumb.h);
            s->position=s->destination=fmaxf(0,fminf(s->maximum,(event->motion.y-s->grab-s->track.y)/range*s->maximum));
            s->elastic=0; return;
        }
    }
    if (event->type==SDL_EVENT_MOUSE_BUTTON_UP) {
        bool scroll_drag=false;
        for (unsigned i=0;i<4;++i) if (d->scrolling[i].dragging) { d->scrolling[i].dragging=false; scroll_drag=true; }
        if (scroll_drag) return;
    }
    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        d->ui.space.mouse_x = event->motion.x; d->ui.space.mouse_y = event->motion.y;
        if (d->dragging) {
            float dx = event->motion.x - d->drag_x, dy = event->motion.y - d->drag_y;
            if (fabsf(dx) + fabsf(dy) > 2) d->moved = true;
            if (SDL_GetModState() & SDL_KMOD_SHIFT) { d->pan_x += dx; d->pan_y += dy; }
            else { d->yaw += dx * 0.007f; d->pitch = fmaxf(-1.2f, fminf(1.2f, d->pitch + dy * 0.007f)); }
            d->drag_x = event->motion.x; d->drag_y = event->motion.y;
            d->view_yaw=d->yaw; d->view_pitch=d->pitch; d->view_pan_x=d->pan_x; d->view_pan_y=d->pan_y;
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        d->keyboard = false;
        for (size_t i = d->target_count; i > 0; --i) if (inside(event->button.x, event->button.y, d->targets[i-1].bounds)) {
            snprintf(d->focus, sizeof(d->focus), "%s", d->targets[i-1].id); break;
        }
        if (map_input(d, event->button.x, event->button.y)) {
            d->dragging = true; d->moved = false; d->drag_x = event->button.x; d->drag_y = event->button.y;
            strcpy(d->focus, "galaxy");
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && d->dragging) {
        d->dragging = false;
        if (!d->moved && event->button.button == SDL_BUTTON_LEFT) {
            float best = 22 * 22; size_t found = d->graph.count;
            for (size_t i = 0; i < d->ui.space.count; ++i) if (d->ui.space.points[i].visible) {
                float dx = d->ui.space.points[i].x - event->button.x, dy = d->ui.space.points[i].y - event->button.y;
                if (dx * dx + dy * dy < best) { best = dx * dx + dy * dy; found = i; }
            }
            if (found < d->model.notes.count) { d->star = found; d->card = true; request(d, SB_ACT_NOTE, d->model.notes.items[found].path); }
        }
    }
    if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        d->heading_cursor[0]=0;
        float delta = event->wheel.y * (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
        if (map_input(d,event->wheel.mouse_x,event->wheel.mouse_y))
            d->zoom = fmaxf(0.45f,fminf(3.5f,d->zoom*expf(delta*0.08f)));
        else {
            d->focus_scroll_frames=0;
            unsigned slot=scroll_slot(d,event->wheel.mouse_x,event->wheel.mouse_y,true);
            if (slot<4) d->scrolling[slot].pending -= delta*50;
        }
        return;
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event->key.key;
#ifdef __APPLE__
        bool modifier = (event->key.mod & SDL_KMOD_GUI) != 0;
#else
        bool modifier = (event->key.mod & SDL_KMOD_CTRL) != 0;
#endif
        bool shift = (event->key.mod & SDL_KMOD_SHIFT) != 0;
        if (d->backup && ((modifier && key==SDLK_Q) || key==SDLK_ESCAPE)) {
            sb_backup_job_cancel(d->backup); if (modifier && key==SDLK_Q) d->quit_after_backup=true; return;
        }
        if (d->backup && (key==SDLK_F1 || key==SDLK_F10 || modifier)) return;
        if (!modifier && !d->model.guard && (key==SDLK_UP || key==SDLK_DOWN) &&
            (d->form==SB_FORM_ACTIONS || d->form==SB_FORM_PROJECTS || d->form==SB_FORM_FILTER || d->form==SB_FORM_SETTINGS)) {
            focus_step(d,key==SDLK_UP ? -1 : 1);
            if (!strcmp(d->focus,"cancel")) focus_step(d,key==SDLK_UP ? -1 : 1);
            return;
        }
        if (key == SDLK_TAB) { focus_step(d, shift ? -1 : 1); return; }
        if (key == SDLK_F6 && d->form == SB_FORM_NONE && !d->model.guard) {
            SBTarget *item = focus_target(d); int group = item ? item->group : 0;
            int next = (group + (shift ? 2 : 1)) % 3;
            if (next == 2) d->card = true;
            if (next == 1) { d->card = false; d->browser = false; }
            focus_set(d, next == 0 ? "project-picker" : next == 1 ? "galaxy" : d->editing ? "editor" : "reader"); return;
        }
        if (key == SDLK_ESCAPE) {
            if (d->model.guard) command(d, SB_CMD_GUARD_CANCEL);
            else if (d->form != SB_FORM_NONE) command(d, SB_CMD_CANCEL);
            else if (d->model.source) { sb_app_source_close(&d->model); d->reset_reader = true; focus_set(d,d->source_focus[0] ? d->source_focus : "reader"); }
            else if (d->search[0]) { d->search[0] = 0; focus_set(d, "search"); }
            else if (d->editing) { d->editing = false; d->expanded=true; focus_set(d, "reader"); }
            else if (d->expanded) { d->expanded=false; d->reset_reader=true; }
            else { d->card = false; focus_set(d, "galaxy"); }
            return;
        }
        if (modifier && key == SDLK_Q) { request(d, SB_ACT_QUIT, NULL); return; }
        if (d->model.guard) {
            if (modifier && key == SDLK_S) { command(d, SB_CMD_GUARD_SAVE); return; }
        } else if (d->form == SB_FORM_NONE && modifier) {
            if (key == SDLK_S) { command(d, shift ? SB_CMD_COPY : SB_CMD_SAVE); return; }
            if (key == SDLK_N) { command(d, shift ? SB_CMD_NEW_PROJECT : SB_CMD_NEW_NOTE); return; }
            if (key == SDLK_O) { command(d, SB_CMD_WORKSPACE); return; }
            if (key == SDLK_F) { d->browser = true; d->expanded=false; focus_set(d, "search"); return; }
            if (key == SDLK_E && d->model.editor && !d->model.source) { d->card = true; d->editing = !d->editing; d->expanded=true; d->focus_editor = d->editing; focus_set(d, d->editing ? "editor" : "reader"); return; }
            if (key == SDLK_R) { command(d, SB_CMD_RELOAD); return; }
            if (key == SDLK_C && shift) { command(d, SB_CMD_CONTEXT); return; }
            if (key == SDLK_COMMA) { d->form = SB_FORM_SETTINGS; return; }
        }
        if (key==SDLK_F10 && shift && !d->model.guard) { d->form=SB_FORM_ACTIONS; return; }
        if (key == SDLK_F1) { d->form = SB_FORM_HELP; return; }
        SBTarget *item = focus_target(d);
        bool text = item && item->kind == SB_FOCUS_TEXT;
        if (text && !strcmp(d->focus, "editor") && key == SDLK_I && (event->key.mod & SDL_KMOD_CTRL)) {
            nk_input_unicode(d->ui.ctx, '\t'); return;
        }
        if (!modifier && !text && (key == SDLK_RETURN || key == SDLK_SPACE)) {
            if (!strcmp(d->focus, "galaxy") && d->star < d->model.notes.count) {
                d->card = true; request(d, SB_ACT_NOTE, d->model.notes.items[d->star].path);
            } else snprintf(d->activate, sizeof(d->activate), "%s", d->focus);
            return;
        }
        if (text && key == SDLK_RETURN && d->form != SB_FORM_NONE) { command(d, SB_CMD_SUBMIT); return; }
        if (!strcmp(d->focus, "galaxy") && !modifier) {
            if (key == SDLK_LEFT || key == SDLK_RIGHT || key == SDLK_UP || key == SDLK_DOWN) {
                if (shift) {
                    if (key == SDLK_LEFT) d->yaw -= 0.1f;
                    if (key == SDLK_RIGHT) d->yaw += 0.1f;
                    if (key == SDLK_UP) d->pitch = fmaxf(-1.2f, d->pitch - 0.1f);
                    if (key == SDLK_DOWN) d->pitch = fminf(1.2f, d->pitch + 0.1f);
                } else {
                    size_t previous = d->star; star_step(d,key);
                    if (d->star != previous && d->star < d->model.notes.count) {
                        d->follow_star = true; d->card = true;
                        request(d,SB_ACT_NOTE,d->model.notes.items[d->star].path);
                    }
                }
                d->keyboard = true; return;
            }
            if (key == SDLK_HOME) { camera_reset(d); return; }
            if (key == SDLK_EQUALS || key == SDLK_PLUS || key == SDLK_KP_PLUS) { d->zoom = fminf(3.5f, d->zoom * 1.15f); return; }
            if (key == SDLK_MINUS || key == SDLK_KP_MINUS) { d->zoom = fmaxf(0.45f, d->zoom / 1.15f); return; }
        }
        if (!modifier && !text && !d->model.guard && focused(d,"reader") && (event->key.mod&SDL_KMOD_ALT) && (key==SDLK_PAGEDOWN || key==SDLK_PAGEUP)) {
            heading_step(d,key==SDLK_PAGEDOWN ? 1 : -1); return;
        }
        if (!text && (key == SDLK_PAGEDOWN || key == SDLK_PAGEUP || key == SDLK_DOWN || key == SDLK_UP)) {
            d->heading_cursor[0]=0; d->focus_scroll_frames = 0;
            unsigned slot=scroll_slot(d,0,0,false);
            if (slot<4) d->scrolling[slot].pending += key == SDLK_DOWN ? 50 : key == SDLK_UP ? -50 : key == SDLK_PAGEDOWN ? 220 : -220;
            return;
        }
    }
    sb_ui_event(&d->ui, event);
}

static void muted(SBDesktop *d, const char *text) {
    passive_add(d,NULL,text,ACCESSKIT_ROLE_LABEL,nk_widget_bounds(d->ui.ctx));
    nk_label_colored(d->ui.ctx, text, NK_TEXT_LEFT,
                     d->ui.contrast ? d->ui.ctx->style.text.color : d->ui.dark ? nk_rgb(164, 169, 181) : nk_rgb(99, 108, 123));
}
static void compact_label(SBDesktop *d, const char *text, char *out, size_t capacity, float width) {
    struct nk_user_font *font = &d->ui.normal->handle;
    snprintf(out, capacity, "%s", text);
    while (out[0] && !sb_utf8_valid(out, strlen(out))) out[strlen(out) - 1] = 0;
    if (font->width(font->userdata, font->height, out, (int)strlen(out)) <= width) return;
    size_t length = strlen(out);
    while (length) {
        --length;
        while (length && ((unsigned char)out[length] & 0xc0) == 0x80) --length;
        out[length] = 0;
        if (length + 4 <= capacity && font->width(font->userdata, font->height, out, (int)length) +
            font->width(font->userdata, font->height, "…", 3) <= width) { strcat(out, "…"); return; }
    }
}
static void native_lines(SBDesktop *d,const char *text) {
    struct nk_context *ctx=d->ui.ctx;
    float width=fmaxf(40,ctx->current->layout->bounds.w-32);
    size_t length=strlen(text),offset=0;
    while (length && !sb_utf8_valid(text,length)) --length;
    while (offset<length) {
        char line[SB_PATH_CAP]; size_t end=offset,next=offset,last_break=offset;
        while (next<length && text[next]!='\n') {
            ++next; while (next<length && ((unsigned char)text[next]&0xc0)==0x80) ++next;
            if (end>offset && d->ui.normal->handle.width(d->ui.normal->handle.userdata,d->ui.normal->handle.height,text+offset,(int)(next-offset))>width) break;
            end=next;
            if (text[end-1]==' ' || text[end-1]=='/') last_break=end;
        }
        if (end<length && text[end]!='\n' && last_break>offset+(end-offset)/2) end=last_break;
        size_t count=end-offset; memcpy(line,text+offset,count); line[count]=0;
        nk_layout_row_dynamic(ctx,22*d->ui.scale,1); native_label(d,line,NK_TEXT_LEFT);
        offset=end; if (offset<length && text[offset]=='\n') ++offset;
    }
}
static void project_problem(SBDesktop *d,const SBProject *project) {
    native_lines(d,project->problem.message);
    native_lines(d,project->root);
    nk_layout_row_dynamic(d->ui.ctx,18*d->ui.scale,1); native_label(d,"Metadaten: brain.json",NK_TEXT_LEFT);
}
static void project_rows(SBDesktop *d) {
    struct nk_context *ctx = d->ui.ctx;
    float scale = d->ui.scale;
    size_t unavailable=0;
    for (size_t i=0;i<d->model.projects.count;++i) if (d->model.projects.items[i].problem.code!=SB_OK) ++unavailable;
    if (unavailable) {
        char summary[100]; snprintf(summary,sizeof(summary),"%zu %s nicht verfügbar. Die Dateien bleiben erhalten.",unavailable,unavailable==1 ? "Projekt ist" : "Projekte sind");
        nk_layout_row_dynamic(ctx,48*scale,1); native_wrap(d,summary);
    }
    size_t start = d->project_page * 6;
    if (start >= d->model.projects.count) { d->project_page = 0; start = 0; }
    for (size_t i = start; i < d->model.projects.count && i < start + 6; ++i) {
        char tag[100];
        nk_bool selected = d->model.has_project && !strcmp(d->model.project.id, d->model.projects.items[i].id);
        nk_layout_row_dynamic(ctx, 36 * scale, 1);
        snprintf(tag, sizeof(tag), "project:%s", d->model.projects.items[i].id);
        if (d->model.projects.items[i].problem.code!=SB_OK) {
            char label[100]; snprintf(label,sizeof(label),"%s · Nicht verfügbar",d->model.projects.items[i].id);
            native_label(d,label,NK_TEXT_LEFT);
            project_problem(d,&d->model.projects.items[i]);
            continue;
        }
        bool duplicate=false;
        for (size_t j=0;j<d->model.projects.count;++j)
            if (i!=j && !strcmp(d->model.projects.items[i].name,d->model.projects.items[j].name)) duplicate=true;
        char label[SB_NAME_CAP],name[181]; snprintf(name,sizeof(name),"%s",d->model.projects.items[i].name);
        while (name[0] && !sb_utf8_valid(name,strlen(name))) name[strlen(name)-1]=0;
        if (duplicate) snprintf(label,sizeof(label),"%s · %s",d->model.projects.items[i].id,name);
        else snprintf(label,sizeof(label),"%s",d->model.projects.items[i].name);
        if (selectable(d, tag, label, &selected))
            request(d, SB_ACT_PROJECT, d->model.projects.items[i].id);
    }
    if (d->model.projects.count > 6) {
        nk_layout_row_dynamic(ctx, 34 * scale, 2);
        if (button(d,"projects-prev","Vorherige Projekte")) d->project_page = d->project_page ? d->project_page-1 : (d->model.projects.count-1)/6;
        if (button(d,"projects-next","Weitere Projekte")) d->project_page = (d->project_page+1)%((d->model.projects.count-1)/6+1);
    }
    nk_layout_row_dynamic(ctx,36*scale,1);
    if (button(d,"refresh-projects","Liste erneut prüfen")) result(d,sb_app_refresh_projects(&d->model),"Projektliste geprüft.");
}
static void glass(SBDesktop *d, struct nk_rect r, float radius) {
    SBSpace *space = &d->ui.space;
    if (space->glass_count < 64) space->glass[space->glass_count++] = (SBGlass){{r.x,r.y,r.w,r.h}, radius};
}
static void search_box(SBDesktop *d) {
    struct nk_context *ctx=d->ui.ctx;
    struct nk_rect box=nk_widget_bounds(ctx);
    struct nk_vec2 padding=ctx->style.window.group_padding, spacing=ctx->style.window.spacing;
    struct nk_style_item background=ctx->style.edit.normal, hover=ctx->style.edit.hover, active=ctx->style.edit.active;
    float border=ctx->style.edit.border;
    nk_fill_rect(nk_window_get_canvas(ctx),box,10,d->ui.dark ? nk_rgba(6,16,30,170) : nk_rgba(255,255,255,180));
    nk_stroke_rect(nk_window_get_canvas(ctx),box,10,1,d->ui.dark ? nk_rgba(160,190,225,70) : nk_rgba(75,100,130,100));
    ctx->style.window.group_padding=nk_vec2(8,0); ctx->style.window.spacing=nk_vec2(2,0);
    ctx->style.edit.normal=ctx->style.edit.hover=ctx->style.edit.active=nk_style_item_color(nk_rgba(0,0,0,0)); ctx->style.edit.border=0;
    if (nk_group_begin(ctx,"Search field",NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_begin(ctx,NK_STATIC,box.h,3);
        nk_layout_row_push(ctx,20*d->ui.scale);
        struct nk_rect icon=nk_widget_bounds(ctx); nk_spacer(ctx);
        sb_icon_draw(nk_window_get_canvas(ctx),SB_ICON_SEARCH,nk_rect(icon.x,icon.y+(icon.h-18*d->ui.scale)/2,18*d->ui.scale,18*d->ui.scale),ctx->style.text.color);
        nk_layout_row_push(ctx,fmaxf(30,box.w-20*d->ui.scale-32*d->ui.scale-24));
        text_target(d,"search"); target_label(d,"Suche im Projekt"); int length=(int)strlen(d->search);
        nk_edit_string(ctx,NK_EDIT_FIELD,d->search,&length,sizeof(d->search),nk_filter_default); d->search[length]=0;
        if (!length && !focused(d,"search")) {
            struct nk_rect hint=d->targets[d->target_count-1].bounds;
            hint.x+=10; hint.w-=20; hint.y+=(hint.h-d->ui.normal->handle.height)/2; hint.h=d->ui.normal->handle.height;
            nk_draw_text(nk_window_get_canvas(ctx),hint,"Suche im Projekt",16,&d->ui.normal->handle,nk_rgba(0,0,0,0),d->ui.dark ? nk_rgb(168,186,210) : nk_rgb(80,100,124));
        }
        ring(d,"search"); nk_layout_row_push(ctx,28*d->ui.scale);
        if (button(d,"clear-search","Suche leeren")) d->search[0]=0;
        nk_layout_row_end(ctx); nk_group_end(ctx);
    }
    ctx->style.window.group_padding=padding; ctx->style.window.spacing=spacing;
    ctx->style.edit.normal=background; ctx->style.edit.hover=hover; ctx->style.edit.active=active; ctx->style.edit.border=border;
}
static void tools(SBDesktop *d, int width, float height, nk_flags flags) {
    struct nk_context *ctx=d->ui.ctx; float s=d->ui.scale;
    struct nk_rect rect=nk_rect(18,16,width-36.0f,height);
    glass(d,rect,18); d->focus_group=0;
    bool compact=width<1180*s;
    if (nk_begin(ctx,"Lumen tools",rect,flags|NK_WINDOW_NO_SCROLLBAR)) {
        float content=ctx->current->layout->bounds.w;
        size_t unavailable=0;
        for (size_t i=0;i<d->model.projects.count;++i) if (d->model.projects.items[i].problem.code!=SB_OK) ++unavailable;
        char project_label[SB_NAME_CAP];
        if (unavailable) snprintf(project_label,sizeof(project_label),"%zu nicht verfügbar · %.180s",unavailable,d->model.project.name);
        else snprintf(project_label,sizeof(project_label),"%s",d->model.has_project ? d->model.project.name : "Projekte");
        while (project_label[0] && !sb_utf8_valid(project_label,strlen(project_label))) project_label[strlen(project_label)-1]=0;
        if (compact) {
            float project=fminf(230*s,content*0.30f), settings=42*s;
            nk_layout_row_begin(ctx,NK_STATIC,36*s,3);
            nk_layout_row_push(ctx,project);
            if (button(d,"project-picker",project_label)) d->form=SB_FORM_PROJECTS;
            nk_layout_row_push(ctx,fmaxf(80,content-project-settings-2*ctx->style.window.spacing.x)); search_box(d);
            nk_layout_row_push(ctx,settings); if (button(d,"settings","Darstellung")) d->form=SB_FORM_SETTINGS;
            nk_layout_row_end(ctx);
            nk_layout_row_begin(ctx,NK_STATIC,36*s,5);
            nk_layout_row_push(ctx,96*s+fmaxf(0,content-326*s-4*ctx->style.window.spacing.x)/2);
        } else {
            float project=fminf(200*s,content*0.18f), fixed=(88+88+112+112+42)*s;
            nk_layout_row_begin(ctx,NK_STATIC,36*s,7);
            nk_layout_row_push(ctx,project);
            if (button(d,"project-picker",project_label)) d->form=SB_FORM_PROJECTS;
            nk_layout_row_push(ctx,fmaxf(90,content-project-fixed-6*ctx->style.window.spacing.x)); search_box(d);
            nk_layout_row_push(ctx,88*s);
        }
        if (button(d,"list","Liste")) { d->browser=d->expanded || !d->browser; d->expanded=false; }
        nk_layout_row_push(ctx,compact ? 104*s+fmaxf(0,content-326*s-4*ctx->style.window.spacing.x)/2 : 88*s);
        int section=0; for (int k=0;k<6;++k) if (!strcmp(d->section,sections[k])) section=k;
        const char *names[]={"Alle","Kern","Wissen","Eingang","Journal","Archiv"};
        if (button(d,"filter",names[section])) d->form=SB_FORM_FILTER;
        nk_layout_row_push(ctx,compact ? 42*s : 112*s);
        if (button(d,"new-note","Notiz")) command(d,SB_CMD_NEW_NOTE);
        nk_layout_row_push(ctx,compact ? 42*s : 112*s);
        if (button(d,"new-project","Projekt")) command(d,SB_CMD_NEW_PROJECT);
        nk_layout_row_push(ctx,42*s);
        if (compact) { if (button(d,"help","Hilfe")) d->form=SB_FORM_HELP; }
        else if (button(d,"settings","Darstellung")) d->form=SB_FORM_SETTINGS;
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
}
static void document_list(SBDesktop *d, struct nk_rect rect, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx; float s = d->ui.scale;
    const SBNotes *notes = d->search[0] ? &d->hits : &d->model.notes;
    glass(d, rect, 16); d->focus_group = 0;
    if (nk_begin(ctx, "Documents", rect, flags | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 20 * s, 1); muted(d, d->search[0] ? "SUCHERGEBNISSE" : "DOKUMENTE");
        if (rect.h < 320*s) {
            int selected = 0;
            for (int k = 0; k < 6; ++k) if (!strcmp(d->section,sections[k])) selected = k;
            nk_layout_row_dynamic(ctx,28*s,1);
            if (button(d,"list-filter",section_names[selected])) d->form = SB_FORM_FILTER;
        } else {
            nk_layout_row_dynamic(ctx, 28 * s, 2);
            for (unsigned k = 0; k < 6; ++k) {
                char tag[100]; snprintf(tag,sizeof(tag),"section:%s",sections[k]);
                nk_bool selected = !strcmp(d->section,sections[k]);
                const char *short_names[] = {"Alle","Kern","Wissen","Eingang","Journal","Archiv"};
                if (selectable(d,tag,short_names[k],&selected)) { strcpy(d->section,sections[k]); d->page = 0; }
            }
        }
        size_t count = 0;
        for (size_t i = 0; i < notes->count; ++i) if (!strcmp(d->section, "all") || !strcmp(d->section, notes->items[i].section)) ++count;
        size_t per_page=(size_t)fmaxf(1,floorf((rect.h-((rect.h<320*s ? 74 : 130)*s+(rect.h<320*s ? 52 : 68)))/(36*s+ctx->style.window.spacing.y)));
        if (d->page * per_page >= count) d->page = 0;
        size_t shown = 0;
        for (size_t i = 0; i < notes->count; ++i) {
            if (strcmp(d->section, "all") && strcmp(d->section, notes->items[i].section)) continue;
            size_t index = shown++;
            if (index < d->page * per_page || index >= (d->page + 1) * per_page) continue;
            char tag[100], title[SB_NAME_CAP];
            if (strlen(notes->items[i].path) < 90) snprintf(tag, sizeof(tag), "note:%s", notes->items[i].path);
            else snprintf(tag, sizeof(tag), "note-hash:%llu", (unsigned long long)sb_hash(notes->items[i].path, strlen(notes->items[i].path)));
            nk_bool selected = !strcmp(d->model.path, notes->items[i].path);
            compact_label(d, notes->items[i].title, title, sizeof(title), rect.w - 60);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (selectable(d, tag, title, &selected)) { d->card = true; request(d, SB_ACT_NOTE, notes->items[i].path); }
            if (nk_widget_is_hovered(ctx)) tooltip(d,notes->items[i].path);
        }
        if (!count) { nk_layout_row_dynamic(ctx, 70 * s, 1); native_wrap(d, "Keine Dokumente in diesem Bereich. Ändere Suche oder Bereich."); }
        if (count > per_page) {
            nk_layout_row_dynamic(ctx, 26 * s, 2);
            if (button(d, "page-prev", "Zurück")) d->page = d->page ? d->page - 1 : (count - 1) / per_page;
            if (button(d, "page-next", "Weiter")) d->page = (d->page + 1) % ((count - 1) / per_page + 1);
        }
    }
    nk_end(ctx);
}


static bool web_reference(const char *destination) {
    const char *schemes[]={"http://","https://"};
    for (size_t k=0;k<2;++k) {
        size_t i=0;
        for (;schemes[k][i];++i) {
            unsigned char c=(unsigned char)destination[i];
            if (c>='A' && c<='Z') c=(unsigned char)(c-'A'+'a');
            if (c!=(unsigned char)schemes[k][i]) break;
        }
        if (!schemes[k][i]) return true;
    }
    return false;
}
static bool mail_reference(const char *destination) {
    const char *scheme="mailto:";
    for (size_t i=0;scheme[i];++i) {
        unsigned char c=(unsigned char)destination[i];
        if (c>='A' && c<='Z') c=(unsigned char)(c-'A'+'a');
        if (c!=(unsigned char)scheme[i]) return false;
    }
    return true;
}
static SBStatus email_uri(const char *destination,char *out,size_t capacity) {
    /* The parser returns the literal CommonMark destination. Escape recipient
       punctuation before handing it to the OS, so '?' and '#' stay in the
       address rather than becoming mail headers or a URI fragment. */
    size_t used=0; const char *hex="0123456789ABCDEF";
    if (capacity<=7) return sb_error(SB_LIMIT,"Die E-Mail-Adresse ist zu lang.");
    memcpy(out,"mailto:",7); used=7;
    for (size_t i=7;destination[i];++i) {
        unsigned char c=(unsigned char)destination[i];
        bool plain=(c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') ||
            c=='-' || c=='.' || c=='_' || c=='~' || c=='@';
        size_t count=plain ? 1 : 3;
        if (count>=capacity-used) { *out=0; return sb_error(SB_LIMIT,"Die E-Mail-Adresse ist zu lang."); }
        if (plain) out[used++]=(char)c;
        else { out[used++]='%'; out[used++]=hex[c>>4]; out[used++]=hex[c&15]; }
    }
    out[used]=0; return sb_ok();
}
static void document_links(SBDesktop *d,SBInline *inline_reader,unsigned *link_number,const char *parent,struct nk_rect *placement) {
    struct nk_context *ctx=d->ui.ctx;
    SBInlineToken link;
    while (sb_inline_next(inline_reader,&link)) {
        if ((link.kind!=SB_INLINE_LINK && link.kind!=SB_INLINE_AUTOLINK) || !link.destination_length) continue;
        char destination[SB_PATH_CAP],label[SB_NAME_CAP],tag[100]; char *full_label=NULL;
        SBStatus status=sb_inline_destination(inline_reader,&link,destination,sizeof(destination));
        if (status.code==SB_OK) status=sb_inline_text(inline_reader,link.content,link.content_length,&full_label);
        if (status.code!=SB_OK) { d->message=status; free(full_label); continue; }
        snprintf(label,sizeof(label),"%s",*full_label ? full_label : destination); free(full_label);
        while (label[0] && !sb_utf8_valid(label,strlen(label))) label[strlen(label)-1]=0;
        nk_style_set_font(ctx,&d->ui.normal->handle);
        if (placement) {
            nk_layout_space_push(ctx,nk_rect(placement->x,placement->y,placement->w,32*d->ui.scale));
            placement->y+=36*d->ui.scale;
        } else nk_layout_row_dynamic(ctx,32*d->ui.scale,1);
        snprintf(tag,sizeof(tag),"link:%u",(*link_number)++);
        size_t before=d->target_count;
        bool pressed=button(d,tag,label);
        if (d->target_count>before && parent) snprintf(d->targets[d->target_count-1].parent,sizeof(d->targets[0].parent),"%s",parent);
        if (pressed) {
            if (web_reference(destination) || mail_reference(destination)) {
                char url[SB_PATH_CAP]; const char *opened=destination;
                if (link.email) {
                    status=email_uri(destination,url,sizeof(url)); opened=url;
                    if (status.code!=SB_OK) { d->message=status; continue; }
                }
                bool success=d->open_url ? d->open_url(opened) : SDL_OpenURL(opened);
                result(d,success ? sb_ok() : sb_error(SB_IO,"%s",SDL_GetError()),
                    mail_reference(destination) ? "E-Mail-App geöffnet." : "Webquelle im Browser geöffnet.");
            }
            else { strcpy(d->command_value,destination); command(d,SB_CMD_SOURCE); }
        }
    }

}
typedef struct { SBInline inline_reader; SBStyledText styled; char *raw,*plain; size_t row_offset; unsigned links; float height; } SBTableViewCell;
static void table_passive(SBDesktop *d,const char *id,const char *text,accesskit_role role,const char *parent,struct nk_rect bounds,float reader_top,unsigned slot,size_t row,size_t column,size_t rows,size_t columns) {
    size_t before=d->passive_count; passive_add(d,id,text,role,bounds);
    if (before==d->passive_count) return;
    SBPassiveText *p=&d->passive[before]; snprintf(p->parent,sizeof(p->parent),"%s",parent);
    p->row=row; p->column=column; p->rows=rows; p->columns=columns;
    p->document_y=fmaxf(0,bounds.y-reader_top+d->scrolling[slot].applied);
    if (!strcmp(d->reveal_document,id) && d->reveal_document_context==accessible_context(d)) {
        d->scrolling[slot].destination=p->document_y; d->scrolling[slot].pending=d->scrolling[slot].elastic=0;
        d->scrolling[slot].active=true; d->reveal_document[0]=0;
    }
}
static bool document_table(SBDesktop *d,const char *text,size_t length,size_t offset,unsigned slot,unsigned *link_number,const SBReferences *references,const SBProjection *projection,const char *parent) {
    SBTable table; if (!sb_table_parse(text,length,offset,&table)) return false;
    size_t count=table.rows*table.columns;
    SBTableViewCell *cells=calloc(count,sizeof(*cells));
    if (!cells) { d->message=sb_error(SB_MEMORY,"Tabelle benötigt mehr Speicher."); return false; }
    size_t cursor=table.offset,row_index=0; SBTableRow row; SBStatus status=sb_ok();
    while (sb_table_next(&table,&cursor,&row)) {
        for (size_t c=0;c<table.columns;++c) {
            SBTableViewCell *cell=&cells[row_index*table.columns+c]; cell->row_offset=row.offset;
            if (projection) { status=sb_projection_source(projection,row.offset,&cell->row_offset); if (status.code!=SB_OK) goto cleanup; }
            status=sb_table_cell_text(&table,&row.cells[c],&cell->raw);
            if (status.code==SB_OK) status=sb_inline_init_references(&cell->inline_reader,cell->raw,strlen(cell->raw),references);
            if (status.code==SB_OK) { status=sb_inline_styled(&cell->inline_reader,0,strlen(cell->raw),&cell->styled); cell->plain=cell->styled.text; }
            if (status.code!=SB_OK) goto cleanup;
            SBInlineToken link; while (sb_inline_next(&cell->inline_reader,&link)) if ((link.kind==SB_INLINE_LINK || link.kind==SB_INLINE_AUTOLINK) && link.destination_length) ++cell->links;
            cell->inline_reader.cursor=0;
        }
        ++row_index;
    }
    struct nk_context *ctx=d->ui.ctx; float s=d->ui.scale,reader_top=ctx->current->layout->clip.y;
    float available=ctx->current->layout->bounds.w;
    bool stacked=available/table.columns<130*s;
    bool compact_header=stacked && table.rows>1;
    for (size_t c=0;c<table.columns;++c) if (cells[c].links) compact_header=false;
    float column_width=stacked ? available : available/table.columns;
    float padding=8*s,text_width=fmaxf(40,column_width-2*padding-4*s);
    const struct nk_user_font *font=&d->ui.body->handle;
    size_t source_offset=offset; if (projection) { status=sb_projection_source(projection,offset,&source_offset); if (status.code!=SB_OK) goto cleanup; }
    offset=source_offset;
    char table_id[100]; snprintf(table_id,sizeof(table_id),"reader:table:%zu",offset);
    size_t table_passive_index=SIZE_MAX; bool table_visible=false;
    for (size_t r=0;r<table.rows;++r) {
        if (!r && compact_header) {
            /* The repeated labels beside each value display the headers. Keep
               logical column headers, without inventing visible rectangles. */
            struct nk_rect bounds=nk_layout_space_rect_to_screen(ctx,nk_rect(0,ctx->current->layout->row.height,0,0));
            size_t before=d->passive_count;
            table_passive(d,table_id,"Tabelle",ACCESSKIT_ROLE_TABLE,parent,bounds,reader_top,slot,0,0,table.rows,table.columns);
            table_passive_index=d->passive_count>before ? before : SIZE_MAX;
            char row_id[100]; snprintf(row_id,sizeof(row_id),"reader:table:%zu:row:%zu",offset,cells[0].row_offset);
            table_passive(d,row_id,"Zeile 1",ACCESSKIT_ROLE_ROW,table_id,bounds,reader_top,slot,0,0,0,0);
            for (size_t c=0;c<table.columns;++c) {
                char cell_id[100]; snprintf(cell_id,sizeof(cell_id),"reader:table:%zu:row:%zu:cell:%zu",offset,cells[c].row_offset,c);
                table_passive(d,cell_id,cells[c].plain,ACCESSKIT_ROLE_COLUMN_HEADER,row_id,bounds,reader_top,slot,0,c,0,0);
            }
            continue;
        }
        float height=0; unsigned widgets=0;
        for (size_t c=0;c<table.columns;++c) {
            SBTableViewCell *cell=&cells[r*table.columns+c];
            cell->height=sb_ui_styled_height(&d->ui,font,&cell->styled,text_width)+cell->links*36*s;
            if (stacked && r && *cells[c].plain) cell->height+=sb_ui_styled_height(&d->ui,&d->ui.normal->handle,&cells[c].styled,text_width);
            height=stacked ? height+cell->height+2*padding : fmaxf(height,cell->height+2*padding);
            widgets+=1+cell->links+(stacked && r && *cells[c].plain ? 1 : 0);
        }
        nk_layout_space_begin(ctx,NK_STATIC,height,(int)widgets);
        struct nk_rect bounds=nk_layout_space_rect_to_screen(ctx,nk_rect(0,0,available,height));
        if (!r) {
            size_t before=d->passive_count;
            table_passive(d,table_id,"Tabelle",ACCESSKIT_ROLE_TABLE,parent,bounds,reader_top,slot,0,0,table.rows,table.columns);
            table_passive_index=d->passive_count>before ? before : SIZE_MAX;
        }
        struct nk_color bg=d->ui.dark ? nk_rgba(140,167,202,r==0 ? 32 : r%2 ? 13 : 4) : nk_rgba(60,90,135,r==0 ? 22 : r%2 ? 12 : 4);
        nk_fill_rect(nk_window_get_canvas(ctx),bounds,0,bg);
        char row_id[100],row_label[40]; snprintf(row_id,sizeof(row_id),"reader:table:%zu:row:%zu",offset,cells[r*table.columns].row_offset);
        snprintf(row_label,sizeof(row_label),"Zeile %zu",r+1);
        size_t row_before=d->passive_count;
        table_passive(d,row_id,row_label,ACCESSKIT_ROLE_ROW,table_id,bounds,reader_top,slot,r,0,0,0);
        if (table_passive_index<d->passive_count && d->passive_count>row_before) {
            struct nk_rect clipped=d->passive[row_before].bounds;
            if (clipped.w>0 && clipped.h>0) {
                SBPassiveText *parent=&d->passive[table_passive_index];
                if (!table_visible) { parent->bounds=clipped; table_visible=true; }
                else {
                    float right=fmaxf(parent->bounds.x+parent->bounds.w,clipped.x+clipped.w);
                    float bottom=fmaxf(parent->bounds.y+parent->bounds.h,clipped.y+clipped.h);
                    parent->bounds.x=fminf(parent->bounds.x,clipped.x); parent->bounds.y=fminf(parent->bounds.y,clipped.y);
                    parent->bounds.w=right-parent->bounds.x; parent->bounds.h=bottom-parent->bounds.y;
                }
            }
        }
        float top=0;
        for (size_t c=0;c<table.columns;++c) {
            SBTableViewCell *cell=&cells[r*table.columns+c]; float x=stacked ? 0 : c*column_width;
            float cell_height=stacked ? cell->height+2*padding : height;
            struct nk_rect cell_bounds=nk_rect(bounds.x+x,bounds.y+top,column_width,cell_height);
            char cell_id[100]; snprintf(cell_id,sizeof(cell_id),"reader:table:%zu:row:%zu:cell:%zu",offset,cell->row_offset,c);
            size_t before=d->passive_count;
            table_passive(d,cell_id,cell->plain,r ? ACCESSKIT_ROLE_CELL : ACCESSKIT_ROLE_COLUMN_HEADER,row_id,cell_bounds,reader_top,slot,r,c,0,0);
            passive_styles(d,before,&cell->styled,font->height);
            float y=top+padding;
            if (stacked && r && *cells[c].plain) {
                nk_style_set_font(ctx,&d->ui.normal->handle);
                float h=sb_ui_styled_height(&d->ui,&d->ui.normal->handle,&cells[c].styled,text_width);
                nk_layout_space_push(ctx,nk_rect(x+padding,y,column_width-2*padding,h)); sb_ui_styled_draw(&d->ui,&d->ui.normal->handle,&cells[c].styled); y+=h;
            }
            nk_style_set_font(ctx,font);
            float h=sb_ui_styled_height(&d->ui,font,&cell->styled,text_width);
            nk_layout_space_push(ctx,nk_rect(x+padding,y,column_width-2*padding,h));
            nk_flags align=stacked ? NK_TEXT_LEFT : table.alignment[c]==SB_TABLE_RIGHT ? NK_TEXT_RIGHT : table.alignment[c]==SB_TABLE_CENTER ? NK_TEXT_CENTERED : NK_TEXT_LEFT;
            sb_ui_styled_aligned(&d->ui,font,&cell->styled,align); y+=h;
            struct nk_rect placement=nk_rect(x+padding,y,column_width-2*padding,32*s);
            document_links(d,&cell->inline_reader,link_number,cell_id,&placement);
            if (stacked) top+=cell_height;
        }
        nk_layout_space_end(ctx);

    }
cleanup:
    for (size_t i=0;i<count;++i) { sb_inline_free(&cells[i].inline_reader); free(cells[i].raw); sb_styled_free(&cells[i].styled); }
    free(cells);
    if (status.code!=SB_OK) { d->message=status; return false; }
    return true;
}
static void document_block(SBDesktop *d,const char *text,size_t length_total,SBMarkdownBlock block,unsigned slot,unsigned *link_number,const SBReferences *references,const SBProjection *projection,const char *parent,bool *first_heading) {
    struct nk_context *ctx=d->ui.ctx; size_t passive_start=d->passive_count;

        if (block.kind==SB_MD_REFERENCE) return;
        if (block.kind==SB_MD_TABLE && document_table(d,text,length_total,block.content,slot,link_number,references,projection,parent)) { *first_heading=false; return; }
        if (block.kind==SB_MD_RULE) {
            nk_layout_row_dynamic(ctx,24*d->ui.scale,1);
            struct nk_rect bounds=nk_widget_bounds(ctx);
            /* Nuklear stores integral stroke widths. Keep an opaque center in
               contrast mode even on a display with one pixel per point. */
            float thickness=d->ui.contrast ? ceilf(fmaxf(d->ui.scale,fmaxf(2,3/fmaxf(1,d->ui.density)))) : d->ui.scale;
            struct nk_rect line=nk_rect(bounds.x,bounds.y+(bounds.h-thickness)/2,bounds.w,thickness);
            nk_spacer(ctx);
            struct nk_color color=ctx->style.text.color;
            if (!d->ui.contrast) color.a=110;
            nk_stroke_line(nk_window_get_canvas(ctx),line.x,line.y+line.h/2,line.x+line.w,line.y+line.h/2,line.h,color);
            document_span(d,"Abschnittstrennung",strlen("Abschnittstrennung"),block.offset,ACCESSKIT_ROLE_SPLITTER,0,line,slot,false);
            if (d->passive_count>passive_start) snprintf(d->passive[d->passive_count-1].parent,sizeof(d->passive[0].parent),"%s",parent);
            *first_heading=false;
            return;
        }
        const char *content=text+block.content;
        size_t length=block.length;
        unsigned heading=block.level;
        bool code=block.kind==SB_MD_CODE;
        if (block.kind==SB_MD_FENCE) return;
        if (block.kind==SB_MD_BLANK || (!length && !heading)) { nk_layout_row_dynamic(ctx,9*d->ui.scale,1); nk_spacer(ctx); }
        else {
            SBInline inline_reader={0}; SBStyledText styled={0}; SBStatus inline_status=sb_ok(); char *plain=NULL;
            if (!code) {
                inline_status=sb_inline_init_references(&inline_reader,content,length,references);
                if (inline_status.code==SB_OK) { inline_status=sb_inline_styled(&inline_reader,0,length,&styled); plain=styled.text; }
                if (inline_status.code!=SB_OK) d->message=inline_status;
            }
            const char *caption=d->model.source ? d->model.source_title : d->model.title;
            bool fixed_title=*first_heading && heading==1 && d->form==SB_FORM_NONE && plain &&
                styled.count==1 && !styled.spans[0].style && !strcmp(plain,caption);
            if (fixed_title) document_span(d,plain,strlen(plain),block.offset,ACCESSKIT_ROLE_HEADING,1,d->reader_title_bounds,slot,true);
            *first_heading=false;
            if (!fixed_title) {
            struct nk_user_font *font = heading ? &d->ui.heading->handle : code ? &d->ui.code->handle : &d->ui.body->handle;
            nk_style_set_font(ctx, font);
            const char *shown = plain ? plain : content;
            size_t shown_length = plain ? strlen(plain) : length;
            float available = fmaxf(60, ctx->current->layout->bounds.w);
            float measured = font->width(font->userdata, font->height, shown, (int)shown_length);
            float lines = measured < available * 0.92f ? 1 : ceilf(measured / available) + 1;
            float text_height=plain ? sb_ui_styled_height(&d->ui,font,&styled,ctx->current->layout->bounds.w) : lines*(font->height+4);
            nk_layout_row_dynamic(ctx,text_height+(heading ? 10 : 0),1);
            size_t before=d->passive_count;
            document_span(d,shown,shown_length,block.offset,heading ? ACCESSKIT_ROLE_HEADING : code ? ACCESSKIT_ROLE_CODE : ACCESSKIT_ROLE_PARAGRAPH,heading,nk_widget_bounds(ctx),slot,false);
            if (plain) passive_styles(d,before,&styled,font->height);
            else if (code && shown_length) {
                SBTextSpan code_span={0,shown_length,SB_TEXT_CODE};
                SBStyledText literal={.text=d->passive_count==before+1 ? d->passive[before].text : "",.spans=&code_span,.count=1};
                passive_styles(d,before,&literal,font->height);
            }
            if (plain) sb_ui_styled_draw(&d->ui,font,&styled); else nk_text_wrap(ctx, shown, (int)shown_length);
            }
            sb_styled_free(&styled);
            if (!code && inline_status.code==SB_OK) document_links(d,&inline_reader,link_number,parent,NULL);
            sb_inline_free(&inline_reader);
        }
    for (size_t i=passive_start;i<d->passive_count;++i) if (!strcmp(d->passive[i].parent,"reader")) snprintf(d->passive[i].parent,sizeof(d->passive[i].parent),"%s",parent);
}

typedef struct { size_t passive,marker; unsigned depth,ordinal; float inset,marker_width; bool marked; } SBDocumentLayout;
static void document_union(SBPassiveText *parent,const SBPassiveText *child) {
    parent->document_y=fminf(parent->document_y,child->document_y);
    if (child->bounds.w<=0 || child->bounds.h<=0) return;
    if (parent->bounds.w<=0 || parent->bounds.h<=0) { parent->bounds=child->bounds; return; }
    float right=fmaxf(parent->bounds.x+parent->bounds.w,child->bounds.x+child->bounds.w);
    float bottom=fmaxf(parent->bounds.y+parent->bounds.h,child->bounds.y+child->bounds.h);
    parent->bounds.x=fminf(parent->bounds.x,child->bounds.x); parent->bounds.y=fminf(parent->bounds.y,child->bounds.y);
    parent->bounds.w=right-parent->bounds.x; parent->bounds.h=bottom-parent->bounds.y;
}
static bool document_tree(SBDesktop *d,const SBDocument *document,const SBReferences *references,unsigned slot,unsigned *links,bool *first_heading) {
    struct nk_context *ctx=d->ui.ctx; struct nk_panel *panel=ctx->current->layout;
    float base_x=panel->at_x,base_width=panel->bounds.w,s=d->ui.scale;
    SBDocumentLayout *layout=calloc(document->count,sizeof(*layout));
    if (!layout) { d->message=sb_error(SB_MEMORY,"Kein Speicher für Dokumentdarstellung."); return false; }
    layout[0].passive=SIZE_MAX;
    for (size_t i=1;i<document->count;++i) {
        const SBDocumentNode *node=&document->nodes[i]; SBDocumentLayout *entry=&layout[i];
        entry->passive=entry->marker=SIZE_MAX; entry->depth=layout[node->parent].depth;
        entry->inset=layout[node->parent].inset;
        if (node->kind==SB_DOC_ITEM || node->kind==SB_DOC_QUOTE) ++entry->depth;
        if (node->kind==SB_DOC_QUOTE) entry->inset+=24*s;
        if (node->kind==SB_DOC_ITEM) entry->inset+=layout[node->parent].marker_width+8*s;
        char parent[100]; snprintf(parent,sizeof(parent),"%s",node->parent && layout[node->parent].passive<d->passive_count ? d->passive[layout[node->parent].passive].id : "reader");
        float inset=fminf(entry->inset,fmaxf(0,base_width-100*s));
        panel->at_x=base_x+inset; panel->bounds.w=base_width-inset;
        struct nk_rect mark_bounds=nk_widget_bounds(ctx);
        if (node->kind==SB_DOC_LIST || node->kind==SB_DOC_ITEM || node->kind==SB_DOC_QUOTE) {
            char id[100]; snprintf(id,sizeof(id),"reader:container:%zu",i);
            const char *label=node->kind==SB_DOC_QUOTE ? "Zitat" : node->kind==SB_DOC_ITEM ? "Listeneintrag" : node->ordered ? "Nummerierte Liste" : "Liste";
            accesskit_role role=node->kind==SB_DOC_QUOTE ? ACCESSKIT_ROLE_BLOCKQUOTE : node->kind==SB_DOC_ITEM ? ACCESSKIT_ROLE_LIST_ITEM : ACCESSKIT_ROLE_LIST;
            size_t before=d->passive_count; passive_add(d,id,label,role,nk_rect(base_x+inset,panel->at_y,0,0));
            if (d->passive_count>before) { entry->passive=before; snprintf(d->passive[before].parent,sizeof(d->passive[before].parent),"%s",parent); d->passive[before].document_y=INFINITY; }
            if (node->kind==SB_DOC_LIST) {
                entry->ordinal=node->start;entry->marker_width=16*s;
                if (node->ordered) { unsigned number=node->start;const struct nk_user_font *font=&d->ui.body->handle;
                    for(size_t child=node->first;child!=SIZE_MAX;child=document->nodes[child].next) {char label[32];snprintf(label,sizeof(label),"%u%c",number++,node->marker);entry->marker_width=fmaxf(entry->marker_width,font->width(font->userdata,font->height,label,(int)strlen(label)));}
                }
            }
            if (node->kind==SB_DOC_ITEM) {
                entry->ordinal=layout[node->parent].ordinal++; char marker_id[100],marker_label[32];
                snprintf(marker_id,sizeof(marker_id),"reader:marker:%zu",i);
                const SBDocumentNode *list=&document->nodes[node->parent];
                if (list->ordered) snprintf(marker_label,sizeof(marker_label),"%u%c",entry->ordinal,list->marker); else strcpy(marker_label,"•");
                size_t marker_before=d->passive_count;passive_add(d,marker_id,marker_label,ACCESSKIT_ROLE_LIST_MARKER,nk_rect(0,0,0,0));
                if (d->passive_count>marker_before && entry->passive<d->passive_count) { entry->marker=marker_before; snprintf(d->passive[marker_before].parent,sizeof(d->passive[marker_before].parent),"%s",d->passive[entry->passive].id); }
            }
            *first_heading=false;
            if (node->kind!=SB_DOC_ITEM || node->first!=SIZE_MAX) continue;
            nk_layout_row_dynamic(ctx,d->ui.body->handle.height+4*s,1); mark_bounds=nk_widget_bounds(ctx); nk_spacer(ctx);
        } else {
            const char *content=NULL;size_t length=0;
            SBStatus status=sb_document_content(node,&content,&length);
            if (status.code!=SB_OK) { d->message=status; continue; }
            if (!length && node->kind!=SB_DOC_RULE) continue;
            /* Compact list paragraphs share normal line spacing; other blocks
               get a small gap without changing native reading order. */
            bool tight=node->parent && document->nodes[node->parent].kind==SB_DOC_ITEM && document->nodes[document->nodes[node->parent].parent].tight;
            if (node->kind!=SB_DOC_RULE && !tight && !*first_heading) { nk_layout_row_dynamic(ctx,8*s,1); nk_spacer(ctx); }
            size_t before=d->passive_count,target_before=d->target_count;
            SBMarkdownBlock block={.kind=node->kind==SB_DOC_HEADING ? SB_MD_HEADING : node->kind==SB_DOC_CODE || node->kind==SB_DOC_RAW ? SB_MD_CODE : node->kind==SB_DOC_RULE ? SB_MD_RULE : node->kind==SB_DOC_TABLE ? SB_MD_TABLE : SB_MD_TEXT,.offset=node->offset,.content=node->content,.length=length,.level=node->level};
            if (node->kind==SB_DOC_CODE || node->kind==SB_DOC_RAW) {
                size_t end=node->content+length;
                for (size_t at=node->content;at<end;) {
                    size_t next=at;while (next<end && node->view.text[next]!='\n') ++next;
                    block.content=at;block.length=next-at;sb_projection_source(&node->view,at,&block.offset);
                    document_block(d,node->view.text,node->view.length,block,slot,links,references,&node->view,parent,first_heading);
                    at=next<end ? next+1 : next;
                }
            } else document_block(d,node->view.text,node->view.length,block,slot,links,references,&node->view,parent,first_heading);
            if (d->passive_count>before) { mark_bounds=d->passive[before].bounds; mark_bounds.y=d->passive[before].document_y+panel->clip.y-d->scrolling[slot].applied; }
            for (size_t p=before;p<d->passive_count;++p) if (!strcmp(d->passive[p].parent,parent) && node->parent && layout[node->parent].passive<d->passive_count) document_union(&d->passive[layout[node->parent].passive],&d->passive[p]);
            for (size_t t=target_before;t<d->target_count;++t) if (node->parent && layout[node->parent].passive<d->passive_count) {
                SBPassiveText child={.bounds=d->targets[t].bounds,.document_y=fmaxf(0,d->targets[t].bounds.y-panel->clip.y+d->scrolling[slot].applied)};
                struct nk_rect clip=panel->clip;float left=fmaxf(child.bounds.x,clip.x),top=fmaxf(child.bounds.y,clip.y);
                child.bounds=nk_rect(left,top,fmaxf(0,fminf(child.bounds.x+child.bounds.w,clip.x+clip.w)-left),fmaxf(0,fminf(child.bounds.y+child.bounds.h,clip.y+clip.h)-top));
                document_union(&d->passive[layout[node->parent].passive],&child);
            }
        }
        /* Each list item gets one marker aligned with its first rendered line. */
        struct nk_rect bounds=mark_bounds;
        for (size_t ancestor=node->kind==SB_DOC_ITEM ? i : node->parent;ancestor;ancestor=document->nodes[ancestor].parent) {
            const SBDocumentNode *item=&document->nodes[ancestor];
            if (item->kind!=SB_DOC_ITEM || layout[ancestor].marked) continue;
            SBDocumentLayout *mark=&layout[ancestor]; mark->marked=true;
            float margin=fminf(mark->inset,fmaxf(0,base_width-100*s));
            float marker_width=layout[item->parent].marker_width;
            struct nk_rect rect=nk_rect(base_x+margin-marker_width-6*s,bounds.y,marker_width,d->ui.body->handle.height+4*s);
            const SBDocumentNode *list=&document->nodes[item->parent]; char label[32];
            if (list->ordered) { snprintf(label,sizeof(label),"%u%c",mark->ordinal,list->marker); nk_draw_text(nk_window_get_canvas(ctx),rect,label,(int)strlen(label),&d->ui.body->handle,nk_rgba(0,0,0,0),ctx->style.text.color); }
            else { strcpy(label,"•"); float diameter=4*s; nk_fill_circle(nk_window_get_canvas(ctx),nk_rect(rect.x+marker_width-6*s,rect.y+(rect.h-diameter)/2,diameter,diameter),ctx->style.text.color); }
            if (mark->marker<d->passive_count && mark->passive<d->passive_count) {
                SBPassiveText *p=&d->passive[mark->marker]; struct nk_rect clip=panel->clip;
                float left=fmaxf(rect.x,clip.x),top=fmaxf(rect.y,clip.y);
                p->bounds=nk_rect(left,top,fmaxf(0,fminf(rect.x+rect.w,clip.x+clip.w)-left),fmaxf(0,fminf(rect.y+rect.h,clip.y+clip.h)-top));
                p->document_y=fmaxf(0,rect.y-panel->clip.y+d->scrolling[slot].applied);document_union(&d->passive[mark->passive],p);
            }
        }
    }
    panel->at_x=base_x;panel->bounds.w=base_width;
    for (size_t i=document->count;i-->1;) {
        size_t child=layout[i].passive,parent=layout[document->nodes[i].parent].passive;
        if (child>=d->passive_count) continue;
        if (parent<d->passive_count) document_union(&d->passive[parent],&d->passive[child]);
        if (document->nodes[i].kind==SB_DOC_QUOTE) { struct nk_rect r=d->passive[child].bounds; if (r.w>0 && r.h>0) { struct nk_color color=ctx->style.text.color; if (!d->ui.contrast) color.a=130; nk_fill_rect(nk_window_get_canvas(ctx),nk_rect(r.x-12*s,r.y,2*s,r.h),s,color); } }
        if (!isfinite(d->passive[child].document_y)) d->passive[child].document_y=0;
        if (d->reveal_document_context==accessible_context(d) && !strcmp(d->reveal_document,d->passive[child].id)) {
            d->scrolling[slot].destination=d->passive[child].document_y;d->scrolling[slot].pending=d->scrolling[slot].elastic=0;d->scrolling[slot].active=true;d->reveal_document[0]=0;
        }
    }
    free(layout);return true;
}

static void document(SBDesktop *d, const char *text, float height) {
    struct nk_context *ctx = d->ui.ctx;
    const char *extension = d->model.source ? strrchr(d->model.source_path, '.') : NULL;
    bool whole_code = text==d->notice || (text == d->model.source && !d->model.source_directory && (!extension || strcmp(extension, ".md")));
    bool first_heading = true;
    unsigned link_number = 0;
    nk_layout_row_dynamic(ctx, height, 1);
    target(d, "reader"); target_label(d,d->form==SB_FORM_NOTICE_TEXT ? sb_notice_name(d->notice_index) : d->form==SB_FORM_CONTEXT ? "Projektkontext" : d->model.source ? d->model.source_title : d->model.title);
    if (d->target_count) d->targets[d->target_count - 1].kind = SB_FOCUS_READER;
    ring(d, "reader");
    struct nk_vec2 previous_padding=ctx->style.window.group_padding;
    ctx->style.window.group_padding=nk_vec2(8*d->ui.scale,10*d->ui.scale);
    if (!nk_group_begin(ctx, "Reader", NK_WINDOW_NO_SCROLLBAR)) { ctx->style.window.group_padding=previous_padding; return; }
    scroll_gutter(d);
    unsigned slot=modal_reader(d) && (text==d->context || text==d->notice) ? 2 : 0;
    smooth_scroll(d,slot,ctx->current->layout->offset_y);
    SBDocument parsed={0}; SBReferences references={0};
    if (!whole_code) {
        SBStatus status=sb_document_init(&parsed,text,strlen(text));
        if (status.code==SB_OK) status=sb_document_references(&parsed,&references);
        if (status.code!=SB_OK) { d->message=status;whole_code=true; }
    }
    if (!whole_code && !document_tree(d,&parsed,&references,slot,&link_number,&first_heading)) whole_code=true;
    if (whole_code) {
        SBMarkdown reader; SBMarkdownBlock block; sb_markdown_init(&reader,text,strlen(text),true);
        while (sb_markdown_next(&reader,&block)) document_block(d,text,strlen(text),block,slot,&link_number,&references,NULL,"reader",&first_heading);
    }
    nk_style_set_font(ctx, &d->ui.normal->handle);
    sb_references_free(&references); sb_document_free(&parsed);
    scroll_measure(d,slot);
    nk_group_end(ctx);
    ctx->style.window.group_padding=previous_padding;
}

static void actions(SBDesktop *d, float width) {
    (void)width;
    if (button(d, "actions", "Aktionen")) d->form = SB_FORM_ACTIONS;
}
static void detail(SBDesktop *d, float x, float y, float width, float height, nk_flags flags) {
    struct nk_context *ctx=d->ui.ctx; float s=d->ui.scale;
    bool compact=height<340*s; struct nk_vec2 spacing=ctx->style.window.spacing;
    if (compact) ctx->style.window.spacing.y=4;
    if (nk_begin(ctx,"Detail",nk_rect(x,y,width,height),flags|NK_WINDOW_NO_SCROLLBAR)) {
        if (d->reset_reader) { nk_group_set_scroll(ctx,"Reader",0,0); memset(&d->scrolling[0],0,sizeof(d->scrolling[0])); d->reset_reader=false; }
        const char *title=d->model.source ? d->model.source_title : d->model.editor ? d->model.title : "Dein Projektwissen";
        float content=ctx->current->layout->bounds.w;
        nk_layout_row_begin(ctx,NK_STATIC,(compact ? 24 : 32)*s,2);
        nk_layout_row_push(ctx,fmaxf(40,content-32*s-ctx->style.window.spacing.x));
        char short_title[SB_NAME_CAP]; compact_label(d,title,short_title,sizeof(short_title),content-44*s);
        d->reader_title_bounds=nk_widget_bounds(ctx);
        nk_label(ctx,short_title,NK_TEXT_LEFT); if (nk_widget_is_hovered(ctx)) tooltip(d,title);
        nk_layout_row_push(ctx,32*s);
        if (button(d,"close-card","Dokument schließen")) { d->card=false; d->expanded=false; focus_set(d,"galaxy"); }
        nk_layout_row_end(ctx);
        if (height>=340*s) {
        nk_layout_row_dynamic(ctx,22*s,1);
        muted(d,d->model.source ? "Quelle · schreibgeschützt" : d->model.editor ? sb_app_dirty(&d->model) ? "Ungespeicherte Änderungen" : "Gespeichert" : "Ein Ort für Ziele, Notizen und Quellen.");
        }
        if (d->model.source) {
            nk_layout_row_dynamic(ctx,(compact ? 28 : 36)*s,2);
            if (button(d,"source-back","Zum Dokument")) { sb_app_source_close(&d->model); d->reset_reader=true; if (d->keyboard) focus_set(d,d->source_focus[0] ? d->source_focus : "reader"); }
            if (button(d,"expand",d->expanded ? "Kleine Ansicht" : "Groß lesen")) { d->expanded=!d->expanded; d->reset_reader=true; }
        } else if (d->model.editor) {
            nk_layout_row_dynamic(ctx,(compact ? 28 : 36)*s,d->editing || sb_app_dirty(&d->model) ? 4 : 3);
            if (d->editing) {
                if (button(d,"read","Lesen")) { d->editing=false; d->expanded=true; }
                if (button(d,"save","Speichern")) command(d,SB_CMD_SAVE);
            } else {
                if (button(d,"edit","Bearbeiten")) { d->editing=true; d->expanded=true; d->focus_editor=true; }
                if (sb_app_dirty(&d->model) && button(d,"save","Speichern")) command(d,SB_CMD_SAVE);
            }
            if (button(d,"expand",d->expanded ? "Klein" : "Groß lesen")) { d->expanded=!d->expanded; d->reset_reader=true; }
            actions(d,width);
        }
        float remaining=ctx->current->layout->bounds.y+ctx->current->layout->bounds.h-(ctx->current->layout->at_y+ctx->current->layout->row.height);
        float body_height=fmaxf(24,remaining-(compact ? 20 : 26)*s-2*ctx->style.window.spacing.y);
        if (d->model.source) document(d,d->model.source,body_height);
        else if (d->model.editor && d->editing) {
            nk_style_set_font(ctx,&d->ui.body->handle);
            nk_layout_row_dynamic(ctx,body_height,1); text_target(d,"editor"); target_label(d,"Dokument bearbeiten");
            if (d->focus_editor) { nk_edit_focus(ctx,NK_EDIT_ALWAYS_INSERT_MODE); d->focus_editor=false; }
            nk_uint scroll=(nk_uint)d->text_edit.scrollbar.y; smooth_scroll(d,1,&scroll); d->text_edit.scrollbar.y=(float)scroll;
            nk_edit_buffer(ctx,NK_EDIT_BOX,&d->text_edit,nk_filter_default);
            if ((nk_uint)d->text_edit.scrollbar.y < d->scrolling[1].applied) {
                d->scrolling[1].maximum=d->text_edit.scrollbar.y; d->scrolling[1].measured=true;
            }
            d->model.editor[d->text_edit.string.buffer.allocated]=0; ring(d,"editor");
            nk_style_set_font(ctx,&d->ui.normal->handle);
        } else if (d->model.editor) document(d,d->model.editor,body_height);
        else {
            nk_layout_row_dynamic(ctx,body_height,1);
            if (nk_group_begin(ctx,"Welcome",NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx,72*s,1); native_wrap(d,"Lege dein erstes Projekt an. Danach kannst du Ziele festhalten, Notizen sammeln und Quellen verbinden.");
                nk_layout_row_dynamic(ctx,38*s,1);
                if (button(d,"new-project-detail","Neues Projekt")) command(d,SB_CMD_NEW_PROJECT);
                if (button(d,"workspace-detail","Arbeitsordner öffnen")) command(d,SB_CMD_WORKSPACE);
                nk_group_end(ctx);
            }
        }
        nk_layout_row_dynamic(ctx,(compact ? 20 : 26)*s,1);
        const char *feedback=d->message.message[0] ? d->message.message : d->model.source ? d->model.source_path : d->model.path;
        char line[SB_NAME_CAP]; compact_label(d,feedback,line,sizeof(line),content-12);
        nk_label_colored(ctx,line,NK_TEXT_LEFT,d->ui.contrast ? ctx->style.text.color : d->message.code==SB_OK ? d->ui.dark ? nk_rgb(170,188,210) : nk_rgb(80,100,125) : nk_rgb(230,105,110));
        if (nk_widget_is_hovered(ctx)) tooltip(d,feedback);
    }
    nk_end(ctx); ctx->style.window.spacing=spacing;
}
static void popup(SBDesktop *d, int width, int height) {
    struct nk_context *ctx = d->ui.ctx;
    float s = d->ui.scale, w = fminf(width - 40.0f, 580 * s), h = fminf(height - 40.0f, 680 * s);
    if (d->model.guard) {
        h = fminf(height - 40.0f, 430 * s);
        glass(d, nk_rect((width-w)/2, (height-h)/2, w, h), 28);
        if (nk_begin(ctx, "Änderungen erhalten", nk_rect((width - w) / 2, (height - h) / 2, w, h),
                     NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
            passive_add(d,"modal-title","Änderungen erhalten",ACCESSKIT_ROLE_HEADING,nk_rect((width-w)/2,(height-h)/2,w,32*s));
            nk_layout_row_dynamic(ctx, 76 * s, 1);
            native_wrap(d, "Dieses Dokument enthält ungespeicherte Änderungen. Speichere sie vor dem Wechsel oder behalte die Bearbeitung bei.");
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "guard-save", "Speichern und weiter")) command(d, SB_CMD_GUARD_SAVE);
            if (button(d, "guard-discard", "Änderungen verwerfen")) command(d, SB_CMD_GUARD_DISCARD);
            if (button(d, "guard-cancel", "Weiter bearbeiten")) command(d, SB_CMD_GUARD_CANCEL);
            if (d->message.code == SB_CONFLICT)
                if (button(d, "guard-copy", "Eigene Fassung als neue Notiz sichern")) command(d, SB_CMD_COPY);
            nk_layout_row_dynamic(ctx, 60 * s, 1); native_wrap(d, d->message.message);
        }
        nk_end(ctx); return;
    }
    if (d->form == SB_FORM_NONE) return;
    const char *title = d->form == SB_FORM_PROJECT ? "Neues Projekt" : d->form == SB_FORM_NOTE ? "Neue Notiz" :
        d->form == SB_FORM_WORKSPACE ? "Arbeitsordner öffnen" : d->form == SB_FORM_SETTINGS ? "Projekte und Darstellung" :
        d->form == SB_FORM_CONTEXT ? "KI-Kontext" : d->form == SB_FORM_ACTIONS ? "Dokumentaktionen" :
        d->form == SB_FORM_PROJECTS ? "Projekt wählen" : d->form == SB_FORM_FILTER ? "Wissensbereich" :
        d->form==SB_FORM_BACKUP ? "Projekt sichern" : d->form==SB_FORM_RESTORE ? "Sicherung wiederherstellen" : d->form==SB_FORM_ABOUT ? "Über SecondBrain" : d->form==SB_FORM_NOTICE_LIST ? "Lizenzen" : d->form==SB_FORM_NOTICE_TEXT ? sb_notice_name(d->notice_index) : "Tastaturhilfe";
    if (d->form==SB_FORM_ACTIONS || d->form==SB_FORM_FILTER) h=fminf(height-40,(6*42+80)*s+56);
    if (d->form==SB_FORM_ABOUT) h=fminf(height-40,380*s+60);
    if (d->form==SB_FORM_WORKSPACE) h=fminf(height-40,300*s+40);
    if (d->form==SB_FORM_BACKUP || d->form==SB_FORM_RESTORE) h=fminf(height-40,(d->form==SB_FORM_RESTORE && d->restore_checked ? 520 : 340)*s+60);
    glass(d,nk_rect((width-w)/2,(height-h)/2,w,h),16);
    if (nk_begin(ctx,title,nk_rect((width-w)/2,(height-h)/2,w,h),NK_WINDOW_NO_SCROLLBAR)) {
        float available=ctx->current->layout->bounds.w;
        nk_layout_row_begin(ctx,NK_STATIC,32*s,2);
        nk_layout_row_push(ctx,available-32*s-ctx->style.window.spacing.x); passive_add(d,"modal-title",title,ACCESSKIT_ROLE_HEADING,nk_widget_bounds(ctx)); nk_label(ctx,title,NK_TEXT_LEFT);
        nk_layout_row_push(ctx,32*s); if (button(d,"cancel","Schließen")) command(d,SB_CMD_CANCEL);
        nk_layout_row_end(ctx);
        bool form=!d->backup && (d->form==SB_FORM_PROJECT || d->form==SB_FORM_NOTE || d->form==SB_FORM_WORKSPACE || d->form==SB_FORM_BACKUP || d->form==SB_FORM_RESTORE);
        float contents=fmaxf(40,h-60-32*s-((form || d->backup) ? 48*s : 0));
        nk_layout_row_dynamic(ctx,contents,1);
        bool group=nk_group_begin(ctx,"Modal contents",NK_WINDOW_NO_SCROLLBAR);
        if (group) {
        scroll_gutter(d);
        if (d->backup_feedback_reset) { *ctx->current->layout->offset_y=0; memset(&d->scrolling[3],0,sizeof(d->scrolling[3])); d->backup_feedback_reset=false; }
        smooth_scroll(d,3,ctx->current->layout->offset_y);
        if (d->backup) {
            static const char *phases[]={"Dateien erfassen","Sicherung schreiben","Sicherung prüfen","Projekt wiederherstellen","Abschließen","Projekt erneut prüfen"};
            SBBackupJobState *state=&d->backup_state;
            nk_layout_row_dynamic(ctx,42*s,1); native_label(d,state->cancel_requested ? "Abbruch wird abgeschlossen …" : phases[state->phase],NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx,20*s,1);
            bool known=state->total && state->phase!=SB_BACKUP_SCAN;
            if (known) {
                nk_size amount=(nk_size)(state->bytes_total ? (double)state->bytes/(double)state->bytes_total*1000 : (double)state->entries/(double)state->total*1000);
                nk_progress(ctx,&amount,1000,NK_FIXED);
            } else {
                struct nk_rect r=nk_widget_bounds(ctx); nk_spacer(ctx);
                r.y+=r.h*0.3f; r.h*=0.4f;
                struct nk_command_buffer *canvas=nk_window_get_canvas(ctx);
                nk_fill_rect(canvas,r,r.h/2,d->ui.dark ? nk_rgb(40,55,76) : nk_rgb(200,213,230));
                float amount=d->reduced_motion ? 0.5f : 0.5f+0.5f*sinf((float)(SDL_GetTicks()%10000)*0.004f);
                r.x+=r.w*0.75f*amount; r.w*=0.25f; nk_fill_rect(canvas,r,r.h/2,nk_rgb(142,191,255));
            }
            char info[150];
            if (known) snprintf(info,sizeof(info),"%zu von %zu Einträgen · %llu von %llu KiB",state->entries,state->total,(unsigned long long)(state->bytes/1024),(unsigned long long)(state->bytes_total/1024));
            else snprintf(info,sizeof(info),"%zu Einträge erfasst",state->entries);
            nk_layout_row_dynamic(ctx,60*s,1); native_wrap(d,info);
            char shown[SB_NAME_CAP]; compact_label(d,state->path,shown,sizeof(shown),w-70);
            nk_layout_row_dynamic(ctx,32*s,1); native_label(d,shown,NK_TEXT_LEFT);
        } else if (d->form==SB_FORM_BACKUP || d->form==SB_FORM_RESTORE) {
            bool save=d->form==SB_FORM_BACKUP;
            if (d->message.code!=SB_OK && d->message.message[0]) {
                const struct nk_user_font *font=ctx->style.font;
                float measured=font->width(font->userdata,font->height,d->message.message,(int)strlen(d->message.message));
                float rows=ceilf(measured/fmaxf(80,ctx->current->layout->bounds.w-20))+1;
                nk_layout_row_dynamic(ctx,rows*(font->height+5),1); native_wrap(d,d->message.message);
                nk_layout_row_dynamic(ctx,36*s,1);
                if (button(d,"backup-copy-error",d->backup_error_copied ? "Meldung kopiert" : "Meldung kopieren")) {
                    d->backup_error_copied=SDL_SetClipboardText(d->message.message); d->backup_clipboard_failed=!d->backup_error_copied;
                }
                if (d->backup_clipboard_failed) { nk_layout_row_dynamic(ctx,36*s,1); native_wrap(d,"Die Zwischenablage ist nicht erreichbar."); }
            }
            nk_layout_row_dynamic(ctx,48*s,1); native_wrap(d,save ? "Sichere Notizen und Anhänge. Offene Änderungen werden vorher gespeichert." : "Stelle das Projekt in einem neuen Ordner wieder her.");
            field(d,"backup-path","Sicherungsdatei",d->backup_path,sizeof(d->backup_path),1);
            if (strcmp(d->checked_backup,d->backup_path)) d->restore_checked=false;
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"backup-choose",save ? "Speicherort wählen" : "Sicherung auswählen")) {
                d->dialog_serial=sb_dialog_backup(d->dialogs,d->ui.window,d->backup_path,save);
                if (!d->dialog_serial) d->message=sb_error(SB_IO,"Dateiauswahl konnte nicht gestartet werden.");
            }
            if (!save && d->restore_checked) {
                nk_layout_row_dynamic(ctx,32*s,1); native_label(d,d->restore_info.name,NK_TEXT_LEFT);
                char info[150]; snprintf(info,sizeof(info),"%zu Dateien · %zu Ordner · %llu KiB",d->restore_info.files,d->restore_info.directories,(unsigned long long)(d->restore_info.bytes/1024));
                nk_layout_row_dynamic(ctx,36*s,1); native_wrap(d,info);
                field(d,"restore-id","Neuer Projektordner",d->restore_id,sizeof(d->restore_id),2);
                nk_layout_row_dynamic(ctx,36*s,1); if (button(d,"backup-recheck","Sicherung erneut prüfen")) command(d,SB_CMD_INSPECT);
            }
            nk_layout_row_dynamic(ctx,32*s,1); native_wrap(d,save ? "Vorhandene Sicherungen werden nicht ersetzt." : "Vorhandene Projektordner bleiben erhalten.");
        } else if (d->form == SB_FORM_PROJECT || d->form == SB_FORM_NOTE) {
            char previous_id[65];
            field(d, "form-name", d->form == SB_FORM_PROJECT ? "Projektname" : "Titel", d->name, sizeof(d->name), 1);
            if (!d->id_manual) sb_app_slug(d->name, d->id, sizeof(d->id));
            strcpy(previous_id, d->id);
            field(d, "form-id", d->form == SB_FORM_PROJECT ? "Ordnername" : "Dateiname", d->id, sizeof(d->id), 2);
            if (strcmp(previous_id, d->id)) d->id_manual = true;
            nk_layout_row_dynamic(ctx, 44 * s, 1);
            native_wrap(d, "Kleinbuchstaben, Zahlen und Bindestriche. Vorhandene Projekte und Notizen bleiben erhalten.");
            if (d->form == SB_FORM_PROJECT)
                field(d, "form-repo", "Projektordner verknüpfen (optional)", d->repository, sizeof(d->repository), 3);
            else {
                nk_layout_row_dynamic(ctx, 24 * s, 1); native_label(d, "Wissensbereich", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 34 * s, 1);
                if (button(d, "section-choice", new_names[d->note_section])) d->note_section = (d->note_section + 1) % 3;
            }
        } else if (d->form == SB_FORM_WORKSPACE) {
            nk_layout_row_dynamic(ctx, 48 * s, 1);
            native_wrap(d, "Wähle den Ordner mit deinen Projektgedächtnissen.");
            field(d, "form-folder", "Arbeitsordner", d->folder, sizeof(d->folder), 1);
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"choose-folder","Ordner auswählen")) {
                d->dialog_serial=sb_dialog_folder(d->dialogs,d->ui.window,d->folder);
                if (!d->dialog_serial) d->message=sb_error(SB_IO,"Ordnerauswahl konnte nicht gestartet werden.");
            }
        } else if (d->form == SB_FORM_FILTER) {
            nk_layout_row_dynamic(ctx,36*s,1);
            for (unsigned k = 0; k < 6; ++k) {
                char tag[100]; snprintf(tag,sizeof(tag),"section:%s",sections[k]);
                nk_bool selected = !strcmp(d->section,sections[k]);
                if (selectable(d,tag,section_names[k],&selected)) { strcpy(d->section,sections[k]); d->page = 0; d->form = SB_FORM_NONE; }
            }
        } else if (d->form == SB_FORM_PROJECTS) {
            project_rows(d);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "new-project-settings", "Neues Projekt")) command(d, SB_CMD_NEW_PROJECT);
            if (button(d, "workspace-settings", "Arbeitsordner öffnen")) command(d, SB_CMD_WORKSPACE);
            if (button(d,"restore-project","Sicherung wiederherstellen")) command(d,SB_CMD_RESTORE);
        } else if (d->form == SB_FORM_ACTIONS) {
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "context", "KI-Kontext")) command(d, SB_CMD_CONTEXT);
            if (button(d, "reload", "Neu laden")) { command(d, SB_CMD_RELOAD); d->form = SB_FORM_NONE; }
            if (d->model.editor && strchr(d->model.path, '/') && strncmp(d->model.path, "archive/", 8))
                if (button(d, "archive", "Archivieren")) { command(d, SB_CMD_ARCHIVE); d->form = SB_FORM_NONE; }
            if (button(d, "save-copy", "Als neue Notiz speichern")) { command(d, SB_CMD_COPY); d->form = SB_FORM_NONE; }
            if (button(d,"backup-project","Projekt sichern")) command(d,SB_CMD_BACKUP);
            if (button(d,"restore-project","Sicherung wiederherstellen")) command(d,SB_CMD_RESTORE);
            if (button(d, "settings-actions", "Darstellung")) d->form = SB_FORM_SETTINGS;
            if (button(d, "help-actions", "Tastaturhilfe")) d->form = SB_FORM_HELP;
        } else if (d->form == SB_FORM_SETTINGS) {
            nk_layout_row_dynamic(ctx, 24 * s, 1); native_label(d, "Projekte", NK_TEXT_LEFT);
            if (button(d, "project-settings", "Projekt wählen")) d->form = SB_FORM_PROJECTS;
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "workspace-settings", "Arbeitsordner öffnen")) command(d, SB_CMD_WORKSPACE);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "new-project-settings", "Neues Projekt")) command(d, SB_CMD_NEW_PROJECT);
            nk_layout_row_dynamic(ctx, 32 * s, 1); native_label(d, "Darstellung", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "theme", d->ui.dark ? "Helle Darstellung" : "Dunkle Darstellung")) command(d, SB_CMD_THEME);
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"system-theme",d->requested_style.follow_theme ? "Eigene Darstellung verwenden" : "Systemdarstellung verwenden")) { SBStyleChoice style=d->requested_style; style.follow_theme=!style.follow_theme; sb_desktop_set_style(d,style); }
            nk_layout_row_dynamic(ctx, 36 * s, 2);
            if (button(d, "font-minus", "Kleinere Schrift")) { d->next_scale = fmaxf(1, d->ui.scale - 0.25f); command(d, SB_CMD_SCALE); }
            if (button(d, "font-plus", "Größere Schrift")) { d->next_scale = fminf(2, d->ui.scale + 0.25f); command(d, SB_CMD_SCALE); }
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (d->system_style.solid || d->system_style.contrast) native_wrap(d,"Transparenz durch Systemeinstellung reduziert.");
            else if (d->ui.contrast) native_wrap(d,"Transparenz bei erhöhtem Kontrast reduziert.");
            else if (button(d, "transparency", d->requested_style.solid ? "Glasdarstellung aktivieren" : "Transparenz reduzieren")) { SBStyleChoice style=d->requested_style; style.solid=!style.solid; sb_desktop_set_style(d,style); }
            nk_layout_row_dynamic(ctx,36*s,1);
            if (d->system_style.motion) native_wrap(d,"Bewegung durch Systemeinstellung reduziert.");
            else if (button(d,"motion",d->requested_style.motion ? "Animationen aktivieren" : "Bewegung reduzieren")) { SBStyleChoice style=d->requested_style; style.motion=!style.motion; sb_desktop_set_style(d,style); }
            nk_layout_row_dynamic(ctx,36*s,1);
            if (d->system_style.contrast) native_wrap(d,"Kontrast durch Systemeinstellung erhöht.");
            else if (button(d,"contrast",d->requested_style.contrast ? "Normalen Kontrast verwenden" : "Kontrast erhöhen")) { SBStyleChoice style=d->requested_style; style.contrast=!style.contrast; sb_desktop_set_style(d,style); }
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"about","Über SecondBrain")) { d->message=sb_ok(); d->form=SB_FORM_ABOUT; }
        } else if (d->form == SB_FORM_ABOUT) {
            char heading[80]; snprintf(heading,sizeof(heading),"Version %s · In Entwicklung",sb_version());
            nk_layout_row_dynamic(ctx,48*s,1); native_wrap(d,heading);
            /* Nuklear wrapping treats a newline as a glyph; lay out each build line. */
            const char *line=strchr(sb_build_info(),'\n');
            if (line) ++line;
            while (line && *line) {
                const char *end=strchr(line,'\n'); char value[180];
                size_t length=end ? (size_t)(end-line) : strlen(line);
                snprintf(value,sizeof(value),"%.*s",(int)length,line);
                nk_layout_row_dynamic(ctx,36*s,1); native_wrap(d,value);
                line=end ? end+1 : NULL;
            }
            nk_layout_row_dynamic(ctx,48*s,1); native_wrap(d,"© 2026 Angelus Vincent · MIT-Lizenz");
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"copy-version",!strcmp(d->message.message,"Versionsinfos kopiert.") ? "Versionsinfos kopiert" : "Versionsinfos kopieren"))
                result(d,SDL_SetClipboardText(sb_build_info()) ? sb_ok() : sb_error(SB_IO,"Die Zwischenablage ist nicht erreichbar."),"Versionsinfos kopiert.");
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"notice-list","Lizenzen")) { d->message=sb_ok(); d->form=SB_FORM_NOTICE_LIST; }
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"about-back","Zurück zu den Einstellungen")) { d->message=sb_ok(); d->form=SB_FORM_SETTINGS; }
        } else if (d->form==SB_FORM_NOTICE_LIST) {
            nk_layout_row_dynamic(ctx,48*s,1); native_wrap(d,"Originaltexte der Anwendung, Schriften und UI-Komponenten.");
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"notices-about","Zurück zu Über SecondBrain")) { d->message=sb_ok(); d->form=SB_FORM_ABOUT; }
            for (size_t i=0;i<sb_notice_count();++i) {
                char id[100]; snprintf(id,sizeof(id),"notice:%zu",i);
                nk_layout_row_dynamic(ctx,42*s,1);
                if (button(d,id,sb_notice_name(i))) {
                    char *text=NULL; d->message=sb_notice_read(d->ui.font_path,i,&text);
                    if (d->message.code==SB_OK) { free(d->notice); d->notice=text; d->notice_index=i; d->form=SB_FORM_NOTICE_TEXT; }
                }
            }
        } else if (d->form==SB_FORM_NOTICE_TEXT) {
            nk_layout_row_dynamic(ctx,36*s,2);
            if (button(d,"notices-back","Zurück zur Übersicht")) { d->message=sb_ok(); d->form=SB_FORM_NOTICE_LIST; }
            if (button(d,"copy-notice",!strcmp(d->message.message,"Lizenztext kopiert.") ? "Lizenztext kopiert" : "Lizenztext kopieren"))
                result(d,SDL_SetClipboardText(d->notice ? d->notice : "") ? sb_ok() : sb_error(SB_IO,"Die Zwischenablage ist nicht erreichbar."),"Lizenztext kopiert.");
            document(d,d->notice ? d->notice : "",fmaxf(40,contents-48*s-44));
            if (d->message.code!=SB_OK) { nk_layout_row_dynamic(ctx,52*s,1); native_wrap(d,d->message.message); }
        } else if (d->form == SB_FORM_CONTEXT) {
            nk_layout_row_dynamic(ctx,24*s,1); muted(d,"Gespeicherte Kerninformationen");
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d,"copy-context",!strcmp(d->message.message,"Kontext kopiert.") ? "Kontext kopiert" : "Kontext kopieren"))
                result(d, SDL_SetClipboardText(d->context ? d->context : "") ? sb_ok() : sb_error(SB_IO, "%s", SDL_GetError()), "Kontext kopiert.");
            document(d, d->context ? d->context : "", fmaxf(40,contents-60*s-44));
        } else {
            const char *help[] = {"Command auf macOS, Control auf Windows und Linux:",
                "N: Neue Notiz · Umschalt+N: Neues Projekt", "S: Speichern · O: Arbeitsordner öffnen",
                "F: Suche · E: Lesen/Bearbeiten", "R: Neu laden · Umschalt+C: KI-Kontext",
                "Tab / Umschalt+Tab: Fokus · Enter/Leertaste: aktivieren",
                "F6: Werkzeuge, Sterne, Dokument · F1: Hilfe",
                "Sterne: Pfeile öffnen die nächste Notiz sofort",
                "Umschalt+Pfeile: Kamera drehen · +/-: Zoom · Pos1: zurück",
                "Ziehen: drehen · Umschalt+Ziehen: verschieben · Mausrad: Zoom",
                "Bild auf/ab: Lesen scrollen · Alt+Bild auf/ab: Abschnitt",
                "Escape: zurück/schließen",
                "Im Editor: A/C/V/X/Z · Y oder Umschalt+Z · Ctrl+I: Tabulator einfügen"};
            for (size_t i = 0; i < sizeof(help) / sizeof(*help); ++i) {
                nk_layout_row_dynamic(ctx, 40 * s, 1); native_wrap(d, help[i]);
            }
        }
        if (d->message.message[0] && d->form!=SB_FORM_ACTIONS && d->form!=SB_FORM_FILTER && d->form!=SB_FORM_CONTEXT && d->form!=SB_FORM_NOTICE_TEXT && d->form!=SB_FORM_BACKUP && d->form!=SB_FORM_RESTORE && (d->form!=SB_FORM_ABOUT || d->message.code!=SB_OK)) {
            nk_layout_row_dynamic(ctx,52*s,1); native_wrap(d,d->message.message);
        }
        scroll_measure(d,3); nk_group_end(ctx);
        }
        if (d->backup) {
            nk_layout_row_dynamic(ctx,36*s,2); nk_spacer(ctx);
            if (button(d,"backup-cancel","Abbrechen")) command(d,SB_CMD_CANCEL);
        } else if (form) {
            nk_layout_row_dynamic(ctx,36*s,2); nk_spacer(ctx);
            const char *submit=d->form==SB_FORM_WORKSPACE ? "Öffnen" : d->form==SB_FORM_BACKUP ? "Sichern" : d->form==SB_FORM_RESTORE ? d->restore_checked ? "Wiederherstellen" : "Prüfen" : "Anlegen";
            if (button(d,"submit",submit)) command(d,SB_CMD_SUBMIT);
        }
    }
    nk_end(ctx);
}
static void graph_refresh(SBDesktop *d) {
    if (!d->graph_dirty) return;
    d->graph_dirty = false;
    if (strcmp(d->graph_project, d->model.project.root)) {
        snprintf(d->graph_project, sizeof(d->graph_project), "%s", d->model.project.root); camera_reset(d);
        d->view_yaw=d->view_pitch=d->view_pan_x=d->view_pan_y=0; d->view_zoom=1;
        d->focus_x=d->focus_y=d->focus_z=0; memset(d->flight_from,0,sizeof(d->flight_from));
        memset(d->flight_to,0,sizeof(d->flight_to)); d->flight=1; d->map_ready=false;
    }
    if (d->model.has_project) {
        SBStatus status = sb_graph_build(&d->model.project, &d->model.notes, &d->graph);
        if (status.code != SB_OK) { sb_graph_free(&d->graph); result(d, status, NULL); }
    } else sb_graph_free(&d->graph);
    free(d->ui.space.points); d->ui.space.points = NULL; d->ui.space.count = 0;
    if (d->graph.count) {
        d->ui.space.points = calloc(d->graph.count, sizeof(*d->ui.space.points));
        if (!d->ui.space.points) { result(d, sb_error(SB_MEMORY, "Kein Speicher für Sterne."), NULL); return; }
        d->ui.space.count = d->graph.count;
    }
    d->ui.space.graph = &d->graph;
    for (size_t i = 0; i < d->model.notes.count; ++i) if (!strcmp(d->model.path, d->model.notes.items[i].path)) d->star = i;
}
static void galaxy(SBDesktop *d, int width, int height, struct nk_rect card, struct nk_rect list) {
    struct nk_context *ctx = d->ui.ctx; SBSpace *space = &d->ui.space;
    float cy = d->map_bounds.y + d->map_bounds.h / 2;
    float left = d->browser ? list.x + list.w + 10 : 24;
    float right = d->card ? card.x - 12 : width - 24.0f;
    if (right - left < 180) { left = 24; right = width - 24.0f; }
    float cx = (left + right) / 2;
    if (d->follow_star && d->navigation.kind==SB_ACT_NONE && !d->model.guard && d->star<d->graph.count) {
        SBStar v=d->graph.stars[d->star];
        d->flight_from[0]=d->focus_x; d->flight_from[1]=d->focus_y; d->flight_from[2]=d->focus_z;
        d->flight_to[0]=v.x; d->flight_to[1]=v.y; d->flight_to[2]=v.z;
        d->flight=0; d->pan_x=d->pan_y=0; d->follow_star=false;
    }
    d->map_target_cx=cx; d->map_target_cy=cy;
    d->map_target_unit=fminf((right-left)/760,d->map_bounds.h/460);
    if (!d->map_ready) { d->map_cx=cx; d->map_cy=cy; d->map_unit=d->map_target_unit; d->map_ready=true; }
    float factor=d->reduced_motion ? 1 : 1-expf(-d->seconds/0.12f);
    d->map_cx=approach(d->map_cx,cx,factor,0.05f); d->map_cy=approach(d->map_cy,cy,factor,0.05f);
    d->map_unit=approach(d->map_unit,d->map_target_unit,factor,0.0001f);
    cx=d->map_cx; cy=d->map_cy;
    float unit=d->map_unit*d->view_zoom;
    space->camera[0]=d->focus_x; space->camera[1]=d->focus_y; space->camera[2]=d->focus_z;
    space->camera[3]=d->view_yaw; space->camera[4]=d->view_pitch;
    space->camera[5]=cx+d->view_pan_x; space->camera[6]=cy+d->view_pan_y; space->camera[7]=unit;
    for (size_t i = 0; i < space->count; ++i) {
        SBStar v = d->graph.stars[i];
        v.x-=d->focus_x; v.y-=d->focus_y; v.z-=d->focus_z;
        float x = v.x * cosf(d->view_yaw) + v.z * sinf(d->view_yaw);
        float z = -v.x * sinf(d->view_yaw) + v.z * cosf(d->view_yaw);
        float y = v.y * cosf(d->view_pitch) - z * sinf(d->view_pitch);
        z = v.y * sinf(d->view_pitch) + z * cosf(d->view_pitch);
        float perspective = 700 / (700 + z);
        SBPoint *p = &space->points[i];
        p->x = cx + x * perspective * unit + d->view_pan_x;
        p->y = cy + y * perspective * unit + d->view_pan_y;
        p->depth = perspective; p->group = v.group;
        p->selected = !strcmp(d->model.path, d->model.notes.items[i].path);
        p->focused = focused(d, "galaxy") && i == d->star;
        p->visible = inside(p->x, p->y, d->map_bounds) &&
            (!strcmp(d->section, "all") || !strcmp(d->section, d->model.notes.items[i].section));
        if (d->search[0]) {
            bool hit = false;
            for (size_t j = 0; j < d->hits.count; ++j) if (!strcmp(d->hits.items[j].path, d->model.notes.items[i].path)) { hit = true; break; }
            p->visible = p->visible && hit;
        }
    }
    d->focus_group = 1;
    if (nk_begin(ctx, "Galaxy", nk_rect(0,0,(float)width,(float)height), NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR)) {
        target_add(d,"galaxy",d->map_bounds,SB_FOCUS_MAP,1);
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        if (focused(d, "galaxy")) nk_stroke_rect(canvas, d->map_bounds, 24, d->ui.contrast ? 2 : 1, d->ui.contrast ? ctx->style.text.color : nk_rgba(142,191,255,90));
        struct nk_rect labels[80]; unsigned label_count = 0;
        for (size_t i = 0; i < space->count; ++i) {
            SBPoint p = space->points[i];
            if (!p.visible || (d->card && inside(p.x,p.y,card)) || (d->browser && inside(p.x,p.y,list))) continue;
            if (p.selected) nk_stroke_circle(canvas, nk_rect(p.x-8,p.y-8,16,16), d->ui.contrast ? 2 : 1, d->ui.contrast ? ctx->style.text.color : nk_rgba(142,191,255,145));
            if (p.focused) nk_stroke_circle(canvas, nk_rect(p.x-13,p.y-13,26,26), 2.5f, d->ui.contrast ? ctx->style.text.color : nk_rgb(190,223,255));
            char title[SB_NAME_CAP]; compact_label(d, d->model.notes.items[i].title, title, sizeof(title), 200);
            float tw = d->ui.normal->handle.width(d->ui.normal->handle.userdata, d->ui.normal->handle.height, title, (int)strlen(title));
            struct nk_rect r = nk_rect(p.x+12,p.y-8,tw+5,24*d->ui.scale);
            if (d->card && r.x < card.x+card.w && r.x+r.w > card.x && r.y < card.y+card.h && r.y+r.h > card.y)
                r.x = p.x-tw-12;
            if (d->browser && r.x < list.x+list.w && r.x+r.w > list.x && r.y < list.y+list.h && r.y+r.h > list.y) continue;
            bool collision = false;
            for (unsigned j = 0; j < label_count; ++j) if (r.x < labels[j].x+labels[j].w && r.x+r.w > labels[j].x && r.y < labels[j].y+labels[j].h && r.y+r.h > labels[j].y) collision = true;
            if ((!collision && label_count < 80) || p.selected || p.focused) {
                if (d->ui.contrast)
                    nk_fill_rect(canvas,r,0,d->ui.dark ? nk_rgb(0,0,0) : nk_rgb(255,255,255));
                nk_draw_text(canvas,r,title,(int)strlen(title),&d->ui.normal->handle,nk_rgba(0,0,0,0),
                    d->ui.contrast ? ctx->style.text.color : d->ui.dark ? nk_rgb(220,233,251) : nk_rgb(40,61,89));
                if (label_count < 80) labels[label_count++] = r;
            }
        }
        if (!space->count && d->model.has_project) nk_draw_text(canvas, nk_rect(60,cy-20,width-120.0f,70),
            "Dein Projektwissen wird hier zur Sternkarte.", (int)strlen("Dein Projektwissen wird hier zur Sternkarte."),
            &d->ui.body->handle,nk_rgba(0,0,0,0),d->ui.contrast ? ctx->style.text.color : d->ui.dark ? nk_rgb(166,185,211) : nk_rgb(70,88,112));
    }
    nk_end(ctx);
}
static void camera_tools(SBDesktop *d, int width, int height, float available, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx; float s = d->ui.scale;
    float w = fminf(available, fminf(width-36.0f,380*s));
    if (w < 220) return;
    struct nk_rect rect = nk_rect(18,height-80*s,w,64*s);
    glass(d,rect,18); d->focus_group = 1;
    struct nk_vec2 padding=ctx->style.window.padding;
    ctx->style.window.padding.y=(rect.h-28*s)/2;
    if (nk_begin(ctx,"Camera",rect, flags | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_begin(ctx,NK_DYNAMIC,28*s,5);
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"zoom-out","Verkleinern")) d->zoom = fmaxf(0.45f,d->zoom/1.15f);
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"zoom-in","Vergrößern")) d->zoom = fminf(3.5f,d->zoom*1.15f);
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"rotate-left","Nach links drehen")) d->yaw -= 0.18f;
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"rotate-right","Nach rechts drehen")) d->yaw += 0.18f;
        nk_layout_row_push(ctx,0.40f);
        if (button(d,"camera-home","Kamera zurücksetzen")) camera_reset(d);
        nk_layout_row_end(ctx);
    }
    nk_end(ctx); ctx->style.window.padding=padding;
}
static void welcome(SBDesktop *d,int width,int height,nk_flags flags) {
    struct nk_context *ctx=d->ui.ctx; float s=d->ui.scale;
    float w=fminf(width-36.0f,620*s),h=fminf(height-36.0f,520*s);
    struct nk_rect bounds=nk_rect((width-w)/2,(height-h)/2,w,h);
    glass(d,bounds,16); d->focus_group=0;
    if (nk_begin(ctx,"Welcome",bounds,flags|NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx,36*s,1);
        passive_add(d,"welcome-title","Dein Projektgedächtnis",ACCESSKIT_ROLE_HEADING,nk_widget_bounds(ctx));
        nk_style_set_font(ctx,&d->ui.heading->handle);
        nk_label(ctx,"Dein Projektgedächtnis",NK_TEXT_LEFT);
        nk_style_set_font(ctx,&d->ui.normal->handle);
        float remaining=ctx->current->layout->bounds.y+ctx->current->layout->bounds.h-
            (ctx->current->layout->at_y+ctx->current->layout->row.height)-ctx->style.window.spacing.y;
        nk_layout_row_dynamic(ctx,fmaxf(24,remaining),1);
        nk_uint sx=0,sy=0; nk_group_get_scroll(ctx,"WelcomeBody",&sx,&sy); smooth_scroll(d,2,&sy);
        nk_group_set_scroll(ctx,"WelcomeBody",sx,sy);
        if (nk_group_begin(ctx,"WelcomeBody",NK_WINDOW_NO_SCROLLBAR)) {
            scroll_gutter(d);
            nk_layout_row_dynamic(ctx,60*s,1);
            native_wrap(d,"Halte Ziele, Notizen und Quellen für jedes Projekt zusammen. Die Dateien bleiben auf deinem Rechner und sind auch für eine KI lesbar.");
            nk_layout_row_dynamic(ctx,36*s,1);
            if (button(d,"new-project","Neues Projekt")) command(d,SB_CMD_NEW_PROJECT);
            if (d->model.projects.count && button(d,"project-settings","Projekte prüfen")) d->form=SB_FORM_PROJECTS;
            if (button(d,"workspace-detail","Arbeitsordner öffnen")) command(d,SB_CMD_WORKSPACE);
            if (button(d,"restore-project","Sicherung wiederherstellen")) command(d,SB_CMD_RESTORE);
            if (button(d,"help-actions","Tastaturhilfe")) d->form=SB_FORM_HELP;
            if (button(d,"settings-actions","Darstellung")) d->form=SB_FORM_SETTINGS;
            nk_layout_row_dynamic(ctx,24*s,1); native_label(d,"Aktueller Arbeitsordner",NK_TEXT_LEFT);
            char path[SB_NAME_CAP]; compact_label(d,d->model.workspace,path,sizeof(path),w-80);
            nk_layout_row_dynamic(ctx,28*s,1);
            passive_add(d,"welcome-workspace",d->model.workspace,ACCESSKIT_ROLE_LABEL,nk_widget_bounds(ctx));
            nk_label(ctx,path,NK_TEXT_LEFT);
            if (nk_widget_is_hovered(ctx)) tooltip(d,d->model.workspace);
            if (d->message.message[0]) {
                nk_layout_row_dynamic(ctx,60*s,1); native_wrap(d,d->message.message);
            }
            scroll_measure(d,2); nk_group_end(ctx);
        }
    }
    nk_end(ctx);
}
void sb_desktop_frame(SBDesktop *d) {
    if (d->reveal_document[0] && (d->reveal_document_context!=accessible_context(d) ||
        (!modal_reader(d) && (!d->card || (d->editing && !d->model.source))))) d->reveal_document[0]=0;
    int width, height; SDL_GetWindowSize(d->ui.window,&width,&height);
    if (width!=d->layout_width || height!=d->layout_height || d->ui.scale!=d->layout_scale) {
        if (d->keyboard) d->focus_scroll_frames=3;
        d->layout_width=width; d->layout_height=height; d->layout_scale=d->ui.scale;
    }
    synchronize(d);
    passive_clear(d); d->semantic_order=0;
    for (unsigned i=0;i<4;++i) d->scrolling[i].used=false;
    if (d->search[0] && strcmp(d->search,d->searched)) { d->browser=true; d->expanded=false; }
    search_refresh(d); graph_refresh(d);
    bool modal = d->form != SB_FORM_NONE || d->model.guard;
    bool before = d->focus_form != SB_FORM_NONE || d->focus_guard;
    bool enter = modal && (!before || d->form != d->focus_form || d->model.guard != d->focus_guard);
    if (enter) memset(d->scrolling,0,sizeof(d->scrolling));
    if (modal && !before) snprintf(d->saved_focus,sizeof(d->saved_focus),"%s",d->focus);
    if (!modal && before) focus_set(d,d->saved_focus[0] ? d->saved_focus : "project-picker");
    if (!modal && d->focus_editor && d->editing) focus_set(d,"editor");
    d->focus_form = d->form; d->focus_guard = d->model.guard;
    float s = d->ui.scale;
    d->ui.ctx->delta_time_seconds=d->seconds;
    bool compact = width < 1180*s;
    float header = compact ? 72*s+40 : 36*s+32;
    float top = header+28, body = height-top-20;
    float card_width = d->editing || d->expanded ? width-36.0f : fminf(540*s,width*0.43f);
    float card_height = d->editing || d->expanded ? body : fminf(body, 570*s);
    struct nk_rect card = nk_rect(width-card_width-18,top+body-card_height,card_width,card_height);
    struct nk_rect list = nk_rect(18,top,fminf(300*s,width*0.32f),body);
    if (d->browser && !d->expanded && card.x < list.x + list.w + 12) { card.x = list.x + list.w + 12; card.w = width-card.x-18; }
    d->map_bounds = nk_rect(18,top,width-36.0f,body-70*s);
    d->target_count = 0; d->ui.space.glass_count = 0;
    d->ui.space.dark = d->ui.dark; d->ui.space.solid = d->solid; d->ui.space.contrast=d->ui.contrast;
    galaxy(d,width,height,card,list);
    nk_flags flags = modal ? NK_WINDOW_NO_INPUT : 0;
    if (d->model.has_project) tools(d,width,header,flags);
    else { d->target_count=0; welcome(d,width,height,flags); }
    if (d->model.has_project && d->browser && !d->expanded) document_list(d,list,flags);
    if (d->model.has_project && d->card) {
        glass(d,card,16); d->focus_group = 2;
        detail(d,card.x,card.y,card.w,card.h,flags);
    }
    if (d->model.has_project && !d->browser && !(d->card && (d->editing || d->expanded))) camera_tools(d,width,height,d->card ? card.x-36 : width-36.0f,flags);
    if (!modal && (d->form!=SB_FORM_NONE || d->model.guard)) {
        /* A button can open its form while this frame is being constructed. */
        snprintf(d->saved_focus,sizeof(d->saved_focus),"%s",d->focus);
        memset(d->scrolling,0,sizeof(d->scrolling));
        enter=true; d->focus_form=d->form; d->focus_guard=d->model.guard;
    }
    if (d->form!=SB_FORM_NONE || d->model.guard) { d->target_count = 0; passive_clear(d); d->focus_group = 3; }
    d->semantic_context=accessible_context(d);
    struct nk_style_item old_background = d->ui.ctx->style.window.fixed_background;
    d->ui.ctx->style.window.fixed_background = nk_style_item_color(d->ui.contrast ? d->ui.dark ? nk_rgb(0,0,0) : nk_rgb(255,255,255) : d->ui.dark ? nk_rgba(17,29,47,245) : nk_rgba(237,245,255,245));
    popup(d,width,height);
    d->ui.ctx->style.window.fixed_background = old_background;
    for (size_t i = 1; i < d->target_count; ++i) {
        SBTarget value = d->targets[i]; size_t j = i;
        while (j && d->targets[j-1].group > value.group) { d->targets[j] = d->targets[j-1]; --j; }
        d->targets[j] = value;
    }
    if (enter && d->target_count) {
        size_t first=0;
        while (first+1<d->target_count && !strcmp(d->targets[first].id,"cancel")) ++first;
        if (d->targets[first].kind == SB_FOCUS_TEXT) {
            /* The form has already activated its first field; keep SDL text input live. */
            snprintf(d->focus,sizeof(d->focus),"%s",d->targets[first].id);
            d->keyboard = true; d->focus_changed = true; d->focus_scroll_frames = 3;
        } else focus_set(d,d->targets[first].id);
    }
    if (!focus_target(d) && d->target_count) focus_set(d,d->targets[0].id);
    d->activate[0] = 0;
    for (unsigned i=0;i<4;++i) if (!d->scrolling[i].used) {
        d->scrolling[i].active=false; d->scrolling[i].pending=0; d->scrolling[i].elastic=0; d->scrolling[i].ready=false;
    }
    if (d->focus_scroll_frames) --d->focus_scroll_frames;
    nk_style_set_font(d->ui.ctx,&d->ui.normal->handle);
    nk_sdl_update_TextInput(d->ui.ctx);
    accessible_publish(d);
}
