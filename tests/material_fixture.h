/* Shared pixel fixture; frame_pixels is owned by the test. */
static SDL_Surface *frame_pixels;
static uint64_t region(SBUi *ui,struct nk_rect area){
    SDL_Surface *pixels=frame_pixels;
    if(!pixels)return 0;int w,h;SDL_GetWindowSize(ui->window,&w,&h);
    int left=(int)ceilf(area.x*pixels->w/w),top=(int)ceilf(area.y*pixels->h/h),right=(int)floorf((area.x+area.w)*pixels->w/w),bottom=(int)floorf((area.y+area.h)*pixels->h/h);
    uint64_t hash=1;if(left<0 || top<0 || right>pixels->w || bottom>pixels->h || right<=left || bottom<=top)hash=0;
    else for(int y=top;y<bottom;++y)hash=hash*1099511628211ULL^sb_hash((char *)pixels->pixels+y*pixels->pitch+left*4,(size_t)(right-left)*4);
    return hash;
}
static void paint(SBUi *ui,struct nk_rect modal,bool background,bool legacy,bool foreground){
    int w,h;SDL_GetWindowSize(ui->window,&w,&h);struct nk_context *ctx=ui->ctx;
    ui->space.glass_count=2;ui->space.glass[0]=(SBGlass){{10,10,w-20,h-20},16};ui->space.glass[1]=(SBGlass){{modal.x,modal.y,modal.w,modal.h},16};
    nk_input_begin(ctx);SDL_Event event;while(SDL_PollEvent(&event)){}nk_input_end(ctx);
    ctx->style.window.fixed_background=nk_style_item_color(nk_rgba(0,0,0,0));
    if(nk_begin(ctx,"Underlay",nk_rect(10,10,w-20,h-20),NK_WINDOW_NO_SCROLLBAR)){
        for(unsigned i=0;i<30;++i){nk_layout_row_dynamic(ctx,25*ui->scale,1);
            if(background)nk_label(ctx,"BACKGROUND WORDS BACKGROUND WORDS BACKGROUND WORDS BACKGROUND WORDS",NK_TEXT_LEFT);else nk_spacer(ctx);}
        if(background)nk_fill_rect(nk_window_get_canvas(ctx),nk_rect(modal.x+1,modal.y+1,3,3),0,nk_rgb(220,50,80));
    }nk_end(ctx);
    if(legacy)ctx->style.window.fixed_background=nk_style_item_color(ui->contrast ? ui->dark ? nk_rgb(0,0,0) : nk_rgb(255,255,255) : ui->dark ? nk_rgba(17,29,47,245) : nk_rgba(237,245,255,245));
    if(nk_begin(ctx,"Foreground",modal,NK_WINDOW_NO_SCROLLBAR)){
        if(!legacy)sb_ui_material_panel(ui,modal,16);
        nk_layout_row_dynamic(ctx,36*ui->scale,1);if(foreground)nk_label(ctx,"Foreground text",NK_TEXT_LEFT);else nk_spacer(ctx);
    }nk_end(ctx);ctx->style.window.fixed_background=nk_style_item_color(nk_rgba(0,0,0,0));

    sb_ui_draw(ui);
    /* Read the current backbuffer before Present; native backbuffers after
       Present are not a retained copy of the frame. */
    SDL_DestroySurface(frame_pixels);SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL);
    frame_pixels=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;SDL_DestroySurface(raw);
    SDL_RenderPresent(ui->renderer);
}
