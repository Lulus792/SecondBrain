static bool plain_native_build(SBTextFace *face,const SBPlainPlan *plan,SBNativeText *out){
    SBGrapheme reader;SBGraphemeBoundary boundary;size_t count=0;
    if(!sb_grapheme_init(&reader,plan->text,plan->length))return false;
    sb_grapheme_next(&reader,&boundary);while(sb_grapheme_next(&reader,&boundary))++count;
    if(!count)count=1;SBNativeCharBox *boxes=calloc(count,sizeof(*boxes));if(!boxes)return false;
    size_t used=0;float density=face->owner->density;bool ok=true;
    for(size_t row=0;row<plan->count && ok;++row){const SBPlainLine *line=&plan->lines[row];
        size_t length=(row+1<plan->count ? plan->lines[row+1].offset : plan->length)-line->offset;
        const char *source=plan->text+line->offset;SBTextParagraph *bidi=NULL;SBShapeParagraph *paragraph=NULL;SBShapeFontSpan *fonts=NULL;size_t font_count=0;SBCaretPlan carets={0};
        ok=sb_bidi_paragraph_create(source,length,SB_BIDI_AUTO_LTR,&bidi).code==SB_OK;
        if(ok && line->shape.length)ok=plain_fonts(face,source,length,&fonts,&font_count) && sb_shape_paragraph_create(bidi,fonts,font_count,&paragraph).code==SB_OK && sb_caret_plan(paragraph,&line->shape,&carets).code==SB_OK;
        float height=fmaxf(face->nk.handle.height,(line->shape.ascent+line->shape.descent)/density),y=line->y/density,edge=0;
        for(size_t i=0;ok && i<carets.cluster_count;++i){SBCaretCluster c=carets.clusters[i];if(used==count){ok=false;break;}boxes[used++]=(SBNativeCharBox){line->offset+c.byte,c.length,c.left/density,y,(c.right-c.left)/density,height,c.level};}
        SBCaret caret;if(sb_caret_find(&carets,line->shape.length,SB_CARET_AFTER,line->shape.base_level,&caret))edge=caret.x/density;
        size_t suffix=line->shape.length,previous=0;
        if(ok)ok=sb_grapheme_init(&reader,source+suffix,length-suffix);
        if(ok){sb_grapheme_next(&reader,&boundary);
            while(sb_grapheme_next(&reader,&boundary)){if(used==count){ok=false;break;}boxes[used++]=(SBNativeCharBox){line->offset+suffix+previous,boundary.byte-previous,edge,y,0,height,line->shape.base_level};previous=boundary.byte;}}
        if(ok && !plan->length && !used)boxes[used++]=(SBNativeCharBox){0,0,0,y,0,height,line->shape.base_level};
        sb_caret_plan_free(&carets);sb_shape_paragraph_free(paragraph);sb_bidi_paragraph_free(bidi);free(fonts);
    }
    if(ok)ok=sb_native_text_geometry(plan->text,plan->length,NULL,0,boxes,used,out).code==SB_OK;free(boxes);
    if(ok && out->count<out->capacity){SBNativeTextRun *compact=realloc(out->runs,out->count*sizeof(*compact));if(compact){out->runs=compact;out->capacity=out->count;}}
    return ok;
}
bool sb_ui_text_command_native(SBUi *ui,const struct nk_command_text *command,SBNativeText *out){
    if(!ui || !command || !command->font || sb_ui_font_owner(command->font)!=ui || !out || command->length<=0 || !command->foreground.a || !isfinite(command->x) || !isfinite(command->y))return false;
    SBTextFace *face=command->font->userdata.ptr;SBTextSystem *system=face->owner;SBPlainPlan scratch={0};
    SBPlainPlan *plan=plain_get(face,command->string,(size_t)command->length,&scratch);if(!plan)return false;
    SBNativeText temporary={0};const SBNativeText *source=&plan->native;bool ok=true;
    if(!source->count){ok=plain_native_build(face,plan,&temporary);source=&temporary;
        size_t bytes=temporary.capacity*sizeof(*temporary.runs);
        if(ok && (plan==&scratch || (plan->bytes<=SB_PLAIN_BYTES && bytes<=SB_PLAIN_BYTES-plan->bytes))){
            SBPlainPlan *entries=system->plain;
            while(plan!=&scratch && system->plain_bytes+bytes>SB_PLAIN_BYTES){SBPlainPlan *oldest=NULL;
                for(unsigned i=0;i<SB_PLAIN_ENTRIES;++i){SBPlainPlan *entry=&entries[i];if(entry!=plan && entry->text && (!oldest || entry->stamp<oldest->stamp))oldest=entry;}
                if(!oldest)break;system->plain_bytes-=oldest->bytes;plain_free(oldest);}
            if(plan==&scratch || system->plain_bytes+bytes<=SB_PLAIN_BYTES){plan->native=temporary;memset(&temporary,0,sizeof(temporary));plan->bytes+=bytes;if(plan!=&scratch)system->plain_bytes+=bytes;source=&plan->native;}
        }
    }
    SBNativeText copy={0};if(ok){copy=*source;copy.capacity=copy.count;copy.runs=malloc(copy.count*sizeof(*copy.runs));ok=copy.runs!=NULL;
        if(ok){memcpy(copy.runs,source->runs,copy.count*sizeof(*copy.runs));for(size_t i=0;i<copy.count;++i)if(copy.runs[i].geometry){copy.runs[i].x+=command->x;copy.runs[i].y+=command->y;}*out=copy;}}
    sb_native_text_free(&temporary);plain_free(&scratch);return ok;
}
