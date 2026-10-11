        /* SecondBrain MIT UI correction: face removal shares the library list
           with FT_Open_Face and other owners' FT_Done_Face calls. */
        SDL_LockMutex(TTF_state.lock);
        FT_Done_Face(font->face);
        SDL_UnlockMutex(TTF_state.lock);
