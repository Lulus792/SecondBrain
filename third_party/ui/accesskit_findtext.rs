    // SecondBrain UI integration; MIT, see the repository LICENSE.
    fn FindText(
        &self,
        text: &BSTR,
        backward: BOOL,
        ignore_case: BOOL,
    ) -> Result<ITextRangeProvider> {
        use windows::Win32::Globalization::{FindStringOrdinal, FIND_FROMEND, FIND_FROMSTART};
        let needle = text.as_wide();
        if needle.is_empty() || needle.len() > i32::MAX as usize
            || String::from_utf16(needle).is_err()
        {
            return Err(invalid_arg());
        }
        self.read(|range| {
            let source: Vec<u16> = range.text().encode_utf16().collect();
            if source.len() > i32::MAX as usize { return Err(invalid_arg()); }
            // UIA represents no match as S_OK with a null output pointer.
            if source.is_empty() { return Err(Error::empty()); }
            let flags = if backward.as_bool() { FIND_FROMEND } else { FIND_FROMSTART };
            let offset = unsafe { FindStringOrdinal(flags, &source, needle, ignore_case.as_bool()) };
            if offset < 0 {
                if unsafe { windows::Win32::Foundation::GetLastError() }.0 != 0 {
                    return Err(Error::from_win32());
                }
                return Err(Error::empty());
            }
            let base = range.start().to_global_utf16_index();
            let start = range.node().text_position_from_global_utf16_index(base + offset as usize)
                .ok_or_else(invalid_arg)?;
            let end = range.node().text_position_from_global_utf16_index(base + offset as usize + needle.len())
                .ok_or_else(invalid_arg)?;
            let mut found = range;
            found.set_start(start);
            found.set_end(end);
            Ok(PlatformRange::new(&self.context, found).into())
        })
    }
