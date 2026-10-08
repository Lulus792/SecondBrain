// SecondBrain UI integration; MIT, see the repository LICENSE.
// AccessKit keeps the tree/lifetime checks; these methods expose its table data.
fn sb_table_role(role: Role) -> bool {
    matches!(role, Role::Table | Role::Grid | Role::TreeGrid)
}

fn sb_table_parent<'a>(node: &NodeRef<'a>) -> Option<NodeRef<'a>> {
    let mut parent = node.parent();
    while let Some(value) = parent {
        if sb_table_role(value.role()) { return Some(value); }
        parent = value.parent();
    }
    None
}

fn sb_table_cells<'a>(node: &NodeRef<'a>) -> impl Iterator<Item = NodeRef<'a>> + use<'a> {
    node.children().filter(|row| row.role() == Role::Row).flat_map(|row| row.children())
}

impl NodeWrapper<'_> {
    fn sb_grid_supported(&self) -> bool {
        sb_table_role(self.node.role()) &&
            self.node.data().row_count().and_then(|v| i32::try_from(v).ok()).is_some() &&
            self.node.data().column_count().and_then(|v| i32::try_from(v).ok()).is_some()
    }
    fn sb_grid_item_supported(&self) -> bool {
        if !matches!(self.node.role(), Role::Cell | Role::GridCell | Role::ColumnHeader | Role::RowHeader) { return false; }
        let Some(table)=sb_table_parent(self.node) else { return false; };
        let data=self.node.data();
        let Some((row,column))=data.row_index().zip(data.column_index()) else { return false; };
        let rows=data.row_span().unwrap_or(1);let columns=data.column_span().unwrap_or(1);
        rows>0 && columns>0 && i32::try_from(rows).is_ok() && i32::try_from(columns).is_ok() &&
            row.checked_add(rows).zip(table.data().row_count()).is_some_and(|(end,count)| end<=count && i32::try_from(count).is_ok()) &&
            column.checked_add(columns).zip(table.data().column_count()).is_some_and(|(end,count)| end<=count && i32::try_from(count).is_ok())
    }
    fn sb_grid_rows(&self) -> i32 { self.node.data().row_count().and_then(|v| v.try_into().ok()).unwrap_or(0) }
    fn sb_grid_columns(&self) -> i32 { self.node.data().column_count().and_then(|v| v.try_into().ok()).unwrap_or(0) }
    fn sb_cell_row(&self) -> i32 { self.node.data().row_index().and_then(|v| v.try_into().ok()).unwrap_or(0) }
    fn sb_cell_column(&self) -> i32 { self.node.data().column_index().and_then(|v| v.try_into().ok()).unwrap_or(0) }
    fn sb_cell_rows(&self) -> i32 { self.node.data().row_span().unwrap_or(1).try_into().unwrap_or(1) }
    fn sb_cell_columns(&self) -> i32 { self.node.data().column_span().unwrap_or(1).try_into().unwrap_or(1) }
    fn sb_table_major(&self) -> RowOrColumnMajor { RowOrColumnMajor_RowMajor }
}

impl PlatformNode_Impl {
    fn sb_headers(&self, columns: bool, item: bool) -> Result<*mut SAFEARRAY> {
        self.resolve_with_context(|node, context| {
            let table = if item { sb_table_parent(&node).ok_or_else(element_not_available)? } else { node };
            let role = if columns { Role::ColumnHeader } else { Role::RowHeader };
            let headers: Vec<IUnknown> = sb_table_cells(&table)
                .filter(|header| header.role() == role && (!item ||
                    if columns { header.data().column_index() == node.data().column_index() }
                    else { header.data().row_index() == node.data().row_index() }))
                .map(|header| context.get_or_create_platform_node(header.id()).into_interface::<IRawElementProviderSimple>().into())
                .collect();
            Ok(safe_array_from_com_slice(&headers))
        })
    }
}

impl From<RowOrColumnMajor> for Variant {
    fn from(value: RowOrColumnMajor) -> Self { Self::from(value.0) }
}
