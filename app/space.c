#include "space.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static float clamp(float x, float a, float b) { return x < a ? a : x > b ? b : x; }
static unsigned char byte(float x) { return (unsigned char)clamp(x, 0, 255); }
static const unsigned char colors[5][3] = {{174,186,255},{139,214,230},{226,195,147},{168,202,225},{132,145,168}};
static void blend(SBSpace *s, int x, int y, const unsigned char *color, float alpha) {
    if (x < 0 || y < 0 || x >= s->width || y >= s->height) return;
    unsigned char *p = s->pixels + ((size_t)y * s->width + x) * 4;
    for (int k = 0; k < 3; ++k) p[k] = byte(p[k] * (1 - alpha) + color[k] * alpha);
}
static void glow(SBSpace *s, float x, float y, float radius, const unsigned char *color, float alpha) {
    int r = (int)ceilf(radius);
    for (int yy = -r; yy <= r; ++yy) for (int xx = -r; xx <= r; ++xx) {
        float fall = 1 - (xx * xx + yy * yy) / (radius * radius);
        if (fall > 0) blend(s, (int)x + xx, (int)y + yy, color, fall * fall * alpha);
    }
}
static void line(SBSpace *s, SBPoint a, SBPoint b) {
    float dx = b.x - a.x, dy = b.y - a.y;
    int steps = (int)fmaxf(fabsf(dx), fabsf(dy));
    if (steps > s->width + s->height || steps < 1) return;
    const unsigned char accent[] = {112,154,197};
    unsigned char ink[3];
    memset(ink, s->dark ? 170 : 85, sizeof(ink));
    const unsigned char *color = s->contrast ? ink : accent;
    for (int i = 0; i <= steps; ++i) {
        int x = (int)(a.x + dx * i / steps), y = (int)(a.y + dy * i / steps);
        blend(s, x, y, color, s->contrast ? 1 : 0.19f);
        if (s->contrast) {
            /* Preserve a visible stroke after the logical raster is scaled. */
            if (fabsf(dx) >= fabsf(dy)) blend(s, x, y + 1, color, 1);
            else blend(s, x + 1, y, color, 1);
        }
    }
}
static void raster_free(SBSpace *s){
    SDL_DestroyTexture(s->texture);s->texture=NULL;
    free(s->pixels);free(s->base);free(s->sky);s->pixels=s->base=s->sky=NULL;
    s->width=s->height=0;s->sky_ready=false;s->cached=false;
}
static bool resize(SBSpace *s,SDL_Renderer *r,int width,int height){
    if(s->width==width && s->height==height && s->texture)return true;
    raster_free(s);
    if(width<1 || height<1 || width>4096 || height>4096)return false;
    size_t bytes=(size_t)width*height*4;
    s->pixels=malloc(bytes);if(!s->pixels)goto fail;
    s->base=malloc(bytes);if(!s->base)goto fail;
    s->sky=malloc(bytes);if(!s->sky)goto fail;
    s->texture=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,width,height);
    if(!s->texture)goto fail;
    s->width=width;s->height=height;return true;
fail:
    raster_free(s);return false;
}
/* A material failure must preserve theme contrast and panel boundaries without
   allocating another CPU raster or texture. UI text is drawn afterwards. */
void sb_space_fallback(const SBSpace *s,SDL_Renderer *renderer){
    unsigned char sky[3]={s->dark ? 7 : 211,s->dark ? 14 : 224,s->dark ? 26 : 239};
    if(s->contrast)memset(sky,s->dark ? 0 : 255,sizeof(sky));
    SDL_SetRenderDrawColor(renderer,sky[0],sky[1],sky[2],255);SDL_RenderClear(renderer);
    SDL_FColor color=s->contrast ? (SDL_FColor){s->dark ? 0 : 1,s->dark ? 0 : 1,s->dark ? 0 : 1,1} :
        s->dark ? (SDL_FColor){17/255.0f,29/255.0f,47/255.0f,1} : (SDL_FColor){237/255.0f,245/255.0f,1,1};
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
    for(unsigned i=0;i<s->glass_count && i<64;++i){SDL_FRect b=s->glass[i].rect;float radius=s->glass[i].radius;
        if(!isfinite(b.x) || !isfinite(b.y) || !isfinite(b.w) || !isfinite(b.h) || !isfinite(radius) || b.w<=0 || b.h<=0)continue;
        radius=clamp(radius,0,fminf(b.w,b.h)/2);
        SDL_Vertex vertices[37];int indices[108];
        vertices[0]=(SDL_Vertex){.position={b.x+b.w/2,b.y+b.h/2},.color=color};
        const float centers[4][2]={{b.x+b.w-radius,b.y+radius},{b.x+b.w-radius,b.y+b.h-radius},{b.x+radius,b.y+b.h-radius},{b.x+radius,b.y+radius}};
        for(unsigned corner=0;corner<4;++corner)for(unsigned step=0;step<9;++step){unsigned at=corner*9+step;
            float angle=((float)corner-1+step/8.0f)*1.5707963267948966f;
            vertices[at+1]=(SDL_Vertex){.position={centers[corner][0]+cosf(angle)*radius,centers[corner][1]+sinf(angle)*radius},.color=color};
            indices[at*3]=0;indices[at*3+1]=(int)at+1;indices[at*3+2]=(int)((at+1)%36)+1;}
        SDL_RenderGeometry(renderer,NULL,vertices,37,indices,108);
    }
}
/* The flat interior has no edge light/bend. Sample columns are invariant for
   the whole pane; keep the exact old float expression when calculating them. */
static void glass_interior(SBSpace *s,int first,int end,int y,const int *columns,int sample_y) {
    unsigned char *out=s->pixels+((size_t)y*s->width+first)*4;
    if(s->contrast || s->solid){
        unsigned char rgba[4]={0,0,0,255};
        if(s->contrast)rgba[0]=rgba[1]=rgba[2]=s->dark ? 0 : 255;
        else{const float tint[]={s->dark ? 49.0f : 235.0f,s->dark ? 67.0f : 243.0f,s->dark ? 93.0f : 252.0f};
            for(int k=0;k<3;++k)rgba[k]=(unsigned char)(tint[k]*(s->dark ? 0.48f : 1));}
        for(int x=first;x<end;++x,out+=4)memcpy(out,rgba,4);
    }else{
        const unsigned char *row=s->base+(size_t)sample_y*s->width*4;
        const float tint[]={s->dark ? 49.0f : 235.0f,s->dark ? 67.0f : 243.0f,s->dark ? 93.0f : 252.0f};
        for(int x=first;x<end;++x,out+=4){const unsigned char *sample=row+(size_t)columns[x]*4;
            for(int k=0;k<3;++k){float substrate=s->dark ? fminf(sample[k],74) : sample[k];out[k]=(unsigned char)(substrate*0.73f+tint[k]*0.27f);}}
    }
}
bool sb_space_draw(SBSpace *s, SDL_Renderer *renderer, int width, int height) {
    /* A bounded logical raster keeps material work independent of Retina density. */
    float scale = fminf(1, fminf(1600.0f / width, 1100.0f / height));
    int w = (int)(width * scale), h = (int)(height * scale);
    if (!resize(s, renderer, w, h)) return false;
    uint64_t fingerprint = sb_hash((const char *)s->points, s->count * sizeof(*s->points));
    fingerprint ^= sb_hash((const char *)s->glass, s->glass_count * sizeof(*s->glass));
    fingerprint ^= sb_hash((const char *)&s->mouse_x, sizeof(s->mouse_x)) << 1;
    fingerprint ^= sb_hash((const char *)&s->mouse_y, sizeof(s->mouse_y)) << 2;
    fingerprint ^= sb_hash((const char *)s->camera,sizeof(s->camera));
    fingerprint ^= (uint64_t)s->dark << 3; fingerprint ^= (uint64_t)s->solid << 4; fingerprint^=(uint64_t)s->contrast<<5;
    if (s->graph) fingerprint ^= sb_hash((const char *)s->graph->edges, s->graph->edge_count * sizeof(*s->graph->edges));
    SDL_FRect rect = {0, 0, (float)width, (float)height};
    if (s->cached && s->fingerprint == fingerprint) return SDL_RenderTexture(renderer, s->texture, NULL, &rect);
    if (!s->sky_ready || s->sky_dark != s->dark || s->sky_contrast!=s->contrast) {
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        float nx = (float)x / w, ny = (float)y / h;
        float haze = fmaxf(0, 1 - ((nx - 0.38f) * (nx - 0.38f) * 2 + (ny - 0.48f) * (ny - 0.48f) * 2));
        unsigned char *p = s->pixels + ((size_t)y * w + x) * 4;
        p[0] = byte((s->dark ? 7 : 211) + haze * (s->dark ? 4 : 9));
        p[1] = byte((s->dark ? 14 : 224) + haze * (s->dark ? 9 : 7));
        p[2] = byte((s->dark ? 26 : 239) + haze * (s->dark ? 17 : 5)); p[3] = 255;
        if (s->contrast) p[0]=p[1]=p[2]=s->dark ? 0 : 255;
    }
    const unsigned char dust[] = {157,182,221};
    for (unsigned i = 0; i < (s->contrast ? 0u : 390u); ++i) {
        unsigned hash = i * 2654435761u + 17u;
        float x = (float)(hash % 10000) / 10000 * w;
        hash = hash * 1664525u + 1013904223u;
        float y = (float)(hash % 10000) / 10000 * h;
        blend(s, (int)x, (int)y, dust, s->dark ? 0.15f + (hash % 20) / 100.0f : 0.3f);
    }
    memcpy(s->sky, s->pixels, (size_t)w * h * 4);
    s->sky_ready = true; s->sky_dark = s->dark; s->sky_contrast=s->contrast;
    } else memcpy(s->pixels, s->sky, (size_t)w * h * 4);
    /* Decorative particles share world coordinates with the knowledge stars. */
    if (!s->contrast && s->camera[7]>0) for (unsigned i=0;i<280;++i) {
        unsigned hash=i*2654435761u+29;
        float x=((hash%10000)/10000.0f-0.5f)*2200-s->camera[0];
        hash=hash*1664525u+1013904223u;
        float y=((hash%10000)/10000.0f-0.5f)*1400-s->camera[1];
        hash=hash*1664525u+1013904223u;
        float z=(hash%10000)/10000.0f*850-180-s->camera[2];
        float xx=x*cosf(s->camera[3])+z*sinf(s->camera[3]);
        z=-x*sinf(s->camera[3])+z*cosf(s->camera[3]);
        float yy=y*cosf(s->camera[4])-z*sinf(s->camera[4]);
        z=y*sinf(s->camera[4])+z*cosf(s->camera[4]);
        if (z<-500) continue;
        float depth=700/(700+z);
        float px=(s->camera[5]+xx*depth*s->camera[7])*scale;
        float py=(s->camera[6]+yy*depth*s->camera[7])*scale;
        const unsigned char color[]={159,184,220};
        glow(s,px,py,clamp(depth*1.2f,0.7f,2.2f)*scale,color,clamp(depth*0.28f,0.12f,0.55f));
    }
    if (s->graph) for (size_t i = 0; i < s->graph->edge_count; ++i) {
        SBEdge edge = s->graph->edges[i];
        if (edge.from < s->count && edge.to < s->count && s->points[edge.from].visible && s->points[edge.to].visible) {
            SBPoint a = s->points[edge.from], b = s->points[edge.to];
            a.x *= scale; a.y *= scale; b.x *= scale; b.y *= scale; line(s, a, b);
        }
    }
    for (size_t i = 0; i < s->count; ++i) if (s->points[i].visible) {
        SBPoint p = s->points[i];
        const unsigned char *c = colors[p.group % 5];
        float x = p.x * scale, y = p.y * scale, radius = (p.selected ? 4.5f : 2.8f) * scale * clamp(p.depth,0.65f,1.6f);
        if (s->contrast) {
            unsigned char ink[3]; memset(ink, s->dark ? 255 : 0, sizeof(ink));
            radius = fmaxf(radius, 2);
            int r = (int)ceilf(radius);
            for (int yy=-r; yy<=r; ++yy) for (int xx=-r; xx<=r; ++xx)
                if (xx*xx+yy*yy <= radius*radius) blend(s,(int)x+xx,(int)y+yy,ink,1);
            continue;
        }
        glow(s, x, y, (p.selected ? 24 : 15) * scale, c, 0.17f);
        glow(s, x, y, radius, c, 1);
        { const unsigned char white[] = {232,247,255}; blend(s, (int)x, (int)y, white, 1); }
    }
    memcpy(s->base, s->pixels, (size_t)w * h * 4);
    for (unsigned i = 0; i < s->glass_count; ++i) {
        SBGlass g = s->glass[i];
        float cx = (g.rect.x + g.rect.w / 2) * scale, cy = (g.rect.y + g.rect.h / 2) * scale;
        float hw = g.rect.w * scale / 2, hh = g.rect.h * scale / 2;
        float radius = fminf(g.radius * scale, fminf(hw, hh));
        int x0 = (int)fmaxf(0, cx - hw - 12), y0 = (int)fmaxf(0, cy - hh - 12);
        int x1 = (int)fminf(w, cx + hw + 12), y1 = (int)fminf(h, cy + hh + 12);
        if(x0>=x1 || y0>=y1)continue;
        int first=x1,end=x1,columns[4096];
        for(int x=x0;x<x1;++x){float dx=x-cx,qx=fabsf(dx)-hw+radius;
            if(qx<=0 && qx-radius<=-18*scale){if(first==x1)first=x;end=x+1;columns[x]=(int)clamp(cx+dx*0.982f,0,w-1);}}
        for (int y = y0; y < y1; ++y) {
            float dy=y-cy,qy=fabsf(dy)-hh+radius;
            bool interior=first<end && qy<=0 && qy-radius<=-18*scale;
            if(interior)glass_interior(s,first,end,y,columns,(int)clamp(cy+dy*0.982f,0,h-1));
            for (int x = x0; x < x1; ++x) {
            if(interior && x==first){x=end-1;continue;}
            float dx = x - cx;
            float qx = fabsf(dx) - hw + radius;
            float ax = fmaxf(qx, 0), ay = fmaxf(qy, 0), length = sqrtf(ax * ax + ay * ay);
            float distance = length + fminf(fmaxf(qx, qy), 0) - radius;
            unsigned char *p = s->pixels + ((size_t)y * w + x) * 4;
            /* Away from the rim, refraction has zero bend and no highlight.
               Preserve that exact result without evaluating edge lighting. */
            if (distance<=-18*scale) {
                if (s->contrast) p[0]=p[1]=p[2]=s->dark ? 0 : 255;
                else {
                    int sx=(int)clamp(cx+dx*0.982f,0,w-1),sy=(int)clamp(cy+dy*0.982f,0,h-1);
                    const unsigned char *sample=s->base+((size_t)sy*w+sx)*4;
                    const float tint[]={s->dark ? 49.0f : 235.0f,s->dark ? 67.0f : 243.0f,s->dark ? 93.0f : 252.0f};
                    for (int k=0;k<3;++k) {
                        float substrate=s->dark ? fminf(sample[k],74) : sample[k];
                        p[k]=(unsigned char)(s->solid ? tint[k]*(s->dark ? 0.48f : 1) : substrate*0.73f+tint[k]*0.27f);
                    }
                }
                continue;
            }
            if (distance > 0) {
                if (s->contrast) continue;
                float shadow = fmaxf(0, 1 - distance / (12 * scale)) * 0.17f;
                for (int k = 0; k < 3; ++k) p[k] = byte(p[k] * (1 - shadow));
                continue;
            }
            float nx = length > 0 ? ax / length : qx > qy ? 1 : 0;
            float ny = length > 0 ? ay / length : qx > qy ? 0 : 1;
            if (dx < 0) nx = -nx;
            if (dy < 0) ny = -ny;
            float edge = clamp(1 + distance / (18 * scale), 0, 1);
            float bend = edge * edge * 9 * scale;
            int sx = (int)clamp(cx + dx * 0.982f - nx * bend, 0, w - 1);
            int sy = (int)clamp(cy + dy * 0.982f - ny * bend, 0, h - 1);
            const unsigned char *sample = s->base + ((size_t)sy * w + sx) * 4;
            float pointer = fmaxf(0, 1 - (fabsf(x - s->mouse_x * scale) + fabsf(y - s->mouse_y * scale)) / (180 * scale));
            float rim = clamp(1 + distance / (1.6f * scale), 0, 1);
            float light = (0.22f - nx * 0.22f - ny * 0.29f + pointer * 0.4f) * rim;
            const float tint[] = {s->dark ? 49.0f : 235.0f, s->dark ? 67.0f : 243.0f, s->dark ? 93.0f : 252.0f};
            for (int k = 0; k < 3; ++k) {
                float substrate=s->dark ? fminf(sample[k],74) : sample[k];
                float color = s->solid ? tint[k] * (s->dark ? 0.48f : 1) : substrate * 0.73f + tint[k] * 0.27f;
                p[k] = byte(color + light * 130 + edge * edge * 3);
                if (s->contrast) p[k]=distance>-1.5f*scale ? s->dark ? 255 : 0 : s->dark ? 0 : 255;
            }
        }
        }
    }
    if (!SDL_UpdateTexture(s->texture, NULL, s->pixels, w * 4)) return false;
    s->fingerprint = fingerprint; s->cached = true;
    return SDL_RenderTexture(renderer, s->texture, NULL, &rect);
}
void sb_space_free(SBSpace *s) {
    raster_free(s);free(s->points);
    memset(s, 0, sizeof(*s));
}
