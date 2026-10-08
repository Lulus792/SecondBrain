    (UIA_GridPatternId, IGridProvider, IGridProvider_Impl, sb_grid_supported, (
        (UIA_GridRowCountPropertyId, RowCount, sb_grid_rows, i32),
        (UIA_GridColumnCountPropertyId, ColumnCount, sb_grid_columns, i32)
    ), (
        fn GetItem(&self, row: i32, column: i32) -> Result<IRawElementProviderSimple> {
            self.resolve_with_context(|node, context| {
                if row < 0 || column < 0 ||
                    node.data().row_count().is_none_or(|count| row as usize >= count) ||
                    node.data().column_count().is_none_or(|count| column as usize >= count) {
                    return Err(E_INVALIDARG.into());
                }
                let cell = sb_table_cells(&node).find(|cell| {
                    let data=cell.data();
                    matches!(cell.role(),Role::Cell | Role::GridCell | Role::ColumnHeader | Role::RowHeader) &&
                    data.row_index().zip(data.column_index()).is_some_and(|(r,c)|
                        row as usize >= r && (row as usize-r) < data.row_span().unwrap_or(1) &&
                        column as usize >= c && (column as usize-c) < data.column_span().unwrap_or(1))
                }).ok_or_else(element_not_available)?;
                Ok(context.get_or_create_platform_node(cell.id()).into_interface())
            })
        }
    )),
    (UIA_GridItemPatternId, IGridItemProvider, IGridItemProvider_Impl, sb_grid_item_supported, (
        (UIA_GridItemRowPropertyId, Row, sb_cell_row, i32),
        (UIA_GridItemColumnPropertyId, Column, sb_cell_column, i32),
        (UIA_GridItemRowSpanPropertyId, RowSpan, sb_cell_rows, i32),
        (UIA_GridItemColumnSpanPropertyId, ColumnSpan, sb_cell_columns, i32)
    ), (
        fn ContainingGrid(&self) -> Result<IRawElementProviderSimple> {
            self.resolve_with_context(|node, context| {
                let table = sb_table_parent(&node).ok_or_else(element_not_available)?;
                Ok(context.get_or_create_platform_node(table.id()).into_interface())
            })
        }
    )),
    (UIA_TablePatternId, ITableProvider, ITableProvider_Impl, sb_grid_supported, (
        (UIA_TableRowOrColumnMajorPropertyId, RowOrColumnMajor, sb_table_major, RowOrColumnMajor)
    ), (
        fn GetColumnHeaders(&self) -> Result<*mut SAFEARRAY> { self.sb_headers(true, false) },
        fn GetRowHeaders(&self) -> Result<*mut SAFEARRAY> { self.sb_headers(false, false) }
    )),
    (UIA_TableItemPatternId, ITableItemProvider, ITableItemProvider_Impl, sb_grid_item_supported, (), (
        fn GetColumnHeaderItems(&self) -> Result<*mut SAFEARRAY> { self.sb_headers(true, true) },
        fn GetRowHeaderItems(&self) -> Result<*mut SAFEARRAY> { self.sb_headers(false, true) }
    )),
