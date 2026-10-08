/* Unformatted UI text uses the same paragraph/script/glyph contract as fields.
   Exact cache keys include the face; all font changes destroy the owning system. */
#define SB_PLAIN_ENTRIES 128
#define SB_PLAIN_BYTES (16u*1024u*1024u)
typedef struct {SBShapedLine shape;size_t offset;float y;} SBPlainLine;
typedef struct {
    char *text;size_t length,bytes,count;unsigned face;uint64_t hash,stamp;
    SBPlainLine *lines;float advance,height;int ascent,descent;
} SBPlainPlan;
static void plain_free(SBPlainPlan *plan){
    for(size_t i=0;i<plan->count;++i)sb_shape_line_free(&plan->lines[i].shape);
    free(plan->lines);free(plan->text);memset(plan,0,sizeof(*plan));
}
static bool plain_fonts(SBTextFace *face,const char *value,size_t length,SBShapeFontSpan **out,size_t *count){
    SBScriptPlan scripts={0};if(sb_script_plan(value,length,&scripts).code!=SB_OK)return false;
    SBShapeFontSpan *spans=NULL;size_t used=0,capacity=0,script=0,previous=0;TTF_Font *last=NULL;
    SBGrapheme reader;SBGraphemeBoundary boundary;
    if(!sb_grapheme_init(&reader,value,length)){sb_script_plan_free(&scripts);return false;}
    sb_grapheme_next(&reader,&boundary);
    while(sb_grapheme_next(&reader,&boundary)){
        while(script+1<scripts.count && scripts.spans[script].byte+scripts.spans[script].length<=previous)++script;
        TTF_Font *font=cluster_font(face,value+previous,boundary.byte-previous,last);
        uint32_t tag=scripts.count ? scripts.spans[script].script : SB_SCRIPT_COMMON;
        if(used && spans[used-1].font==font && spans[used-1].script==tag)spans[used-1].length+=boundary.byte-previous;
        else{
            if(used==capacity){size_t next=capacity ? capacity*2 : 16;SBShapeFontSpan *grown=realloc(spans,next*sizeof(*grown));if(!grown){free(spans);sb_script_plan_free(&scripts);return false;}spans=grown;capacity=next;}
            spans[used++]=(SBShapeFontSpan){previous,boundary.byte-previous,font,tag};
        }
        previous=boundary.byte;last=font;
    }
    sb_script_plan_free(&scripts);*out=spans;*count=used;return true;
}
static bool plain_create(SBTextFace *face,const char *value,size_t length,SBPlainPlan *out){
    SBPlainPlan plan={.length=length,.face=(unsigned)(face-face->owner->faces),.hash=sb_hash(value,length)};
    plan.text=malloc(length+1);if(!plan.text)return false;memcpy(plan.text,value,length);plan.text[length]=0;
    plan.bytes=sizeof(plan)+length+1;
    size_t offset=0;
    do{
        SBTextParagraph *bidi=NULL;SBShapeFontSpan *fonts=NULL;size_t count=0;
        SBShapedLine shape={0};
        bool success=sb_bidi_paragraph_create(value+offset,length-offset,SB_BIDI_AUTO_LTR,&bidi).code==SB_OK;
        size_t consumed=success ? sb_bidi_paragraph_length(bidi) : 0,visible=consumed;
        /* CRLF and other paragraph separators contribute no displayed ink. */
        if(success && consumed){const char *at=value+offset,*end=at+consumed;size_t last=0;
            while(at<end){last=(size_t)(at-(value+offset));uint32_t cp=SDL_StepUTF8(&at,NULL);if(sb_bidi_separator(cp)){visible=last;break;}}
        }
        success=success && plain_fonts(face,value+offset,consumed,&fonts,&count) && sb_shape_line(bidi,0,visible,fonts,count,&shape).code==SB_OK;
        free(fonts);sb_bidi_paragraph_free(bidi);
        if(!success){sb_shape_line_free(&shape);plain_free(&plan);return false;}
        SBPlainLine *grown=realloc(plan.lines,(plan.count+1)*sizeof(*grown));
        if(!grown){sb_shape_line_free(&shape);plain_free(&plan);return false;}plan.lines=grown;
        plan.lines[plan.count++]=(SBPlainLine){shape,offset,plan.height};
        plan.advance=fmaxf(plan.advance,shape.advance);plan.ascent=SDL_max(plan.ascent,shape.ascent);plan.descent=SDL_max(plan.descent,shape.descent);
        plan.height+=fmaxf(face->nk.handle.height*face->owner->density,shape.ascent+shape.descent);
        plan.bytes+=sizeof(*grown)+shape.capacity*sizeof(*shape.glyphs)+shape.run_capacity*sizeof(*shape.runs);
        offset+=consumed;
        if(!consumed)break;
    }while(offset<length);
    *out=plan;return true;
}
static SBPlainPlan *plain_get(SBTextFace *face,const char *value,size_t length,SBPlainPlan *scratch){
    SBTextSystem *system=face->owner;unsigned number=(unsigned)(face-system->faces);uint64_t hash=sb_hash(value,length);
    SBPlainPlan *entries=system->plain;
    for(unsigned i=0;entries && i<SB_PLAIN_ENTRIES;++i)if(entries[i].text && entries[i].face==number && entries[i].hash==hash && entries[i].length==length && !memcmp(entries[i].text,value,length)){entries[i].stamp=system->frame;return &entries[i];}
    if(!plain_create(face,value,length,scratch))return NULL;
    if(scratch->bytes>SB_PLAIN_BYTES)return scratch;
    if(!entries){entries=calloc(SB_PLAIN_ENTRIES,sizeof(*entries));if(!entries)return scratch;system->plain=entries;}
    unsigned slot=0;for(unsigned i=0;i<SB_PLAIN_ENTRIES;++i){if(!entries[i].text){slot=i;break;}if(entries[i].stamp<entries[slot].stamp)slot=i;}
    while(system->plain_bytes+scratch->bytes-entries[slot].bytes>SB_PLAIN_BYTES){unsigned oldest=SB_PLAIN_ENTRIES;
        for(unsigned i=0;i<SB_PLAIN_ENTRIES;++i)if(i!=slot && entries[i].text && (oldest==SB_PLAIN_ENTRIES || entries[i].stamp<entries[oldest].stamp))oldest=i;
        if(oldest==SB_PLAIN_ENTRIES)break;system->plain_bytes-=entries[oldest].bytes;plain_free(&entries[oldest]);
    }
    system->plain_bytes-=entries[slot].bytes;plain_free(&entries[slot]);entries[slot]=*scratch;memset(scratch,0,sizeof(*scratch));
    system->plain_bytes+=entries[slot].bytes;entries[slot].stamp=system->frame;return &entries[slot];
}
