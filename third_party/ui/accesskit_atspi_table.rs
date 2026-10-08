// SecondBrain UI integration; MIT, see the repository LICENSE.
fn sb_table_role(role: Role) -> bool { matches!(role,Role::Table | Role::Grid | Role::TreeGrid) }
fn sb_table_parent<'a>(node: &NodeRef<'a>) -> Option<NodeRef<'a>> {
    let mut parent=node.parent();
    while let Some(value)=parent { if sb_table_role(value.role()) { return Some(value); } parent=value.parent(); }
    None
}
fn sb_table_shape(node: &NodeRef) -> Result<(usize,usize)> {
    if !sb_table_role(node.role()) { return Err(Error::UnsupportedInterface); }
    let (rows,columns)=node.data().row_count().zip(node.data().column_count()).ok_or(Error::UnsupportedInterface)?;
    i32::try_from(rows).map_err(|_|Error::TooManyChildren)?;i32::try_from(columns).map_err(|_|Error::TooManyChildren)?;
    Ok((rows,columns))
}
fn sb_cell_shape(node: &NodeRef) -> Result<(usize,usize,usize,usize)> {
    if !matches!(node.role(),Role::Cell | Role::GridCell | Role::ColumnHeader | Role::RowHeader) { return Err(Error::UnsupportedInterface); }
    let table=sb_table_parent(node).ok_or(Error::UnsupportedInterface)?;let (rows,columns)=sb_table_shape(&table)?;
    let data=node.data();let (row,column)=data.row_index().zip(data.column_index()).ok_or(Error::UnsupportedInterface)?;
    let rs=data.row_span().unwrap_or(1);let cs=data.column_span().unwrap_or(1);
    if rs==0 || cs==0 || row.checked_add(rs).is_none_or(|end|end>rows) || column.checked_add(cs).is_none_or(|end|end>columns) { return Err(Error::IndexOutOfRange); }
    Ok((row,column,rs,cs))
}
fn sb_table_cells<'a>(table:&NodeRef<'a>) -> impl Iterator<Item=NodeRef<'a>> + use<'a> { table.filtered_children(&filter) }
fn sb_table_at<'a>(table:&NodeRef<'a>,row:usize,column:usize) -> Option<NodeRef<'a>> {
    sb_table_cells(table).find(|cell|sb_cell_shape(cell).is_ok_and(|(r,c,rs,cs)|row>=r && row-r<rs && column>=c && column-c<cs))
}
impl NodeWrapper<'_> {
    fn sb_supports_table(&self)->bool { sb_table_shape(self.0).is_ok() }
    fn sb_supports_table_cell(&self)->bool { sb_cell_shape(self.0).is_ok() }
}
impl PlatformNode {
    pub fn sb_table_dimensions(&self)->Result<(i32,i32)> { self.resolve(|node|sb_table_shape(&node).map(|(r,c)|(r as i32,c as i32))) }
    pub fn sb_table_cell_at(&self,row:i32,column:i32)->Result<Option<FullNodeId>> {
        self.resolve(|node| { let (rows,columns)=sb_table_shape(&node)?;
            if row<0 || column<0 || row as usize>=rows || column as usize>=columns { return Ok(None); }
            Ok(sb_table_at(&node,row as usize,column as usize).map(|cell|cell.id())) })
    }
    pub fn sb_table_index_at(&self,row:i32,column:i32)->Result<i32> {
        self.resolve(|node| { sb_table_shape(&node)?;if row<0 || column<0 { return Ok(-1); }
            let Some(cell)=sb_table_at(&node,row as usize,column as usize) else { return Ok(-1); };
            let index=sb_table_cells(&node).position(|value|value.id()==cell.id()).ok_or(Error::IndexOutOfRange)?;
            i32::try_from(index).map_err(|_|Error::TooManyChildren) })
    }
    pub fn sb_table_extents_at_index(&self,index:i32)->Result<(bool,i32,i32,i32,i32,bool)> {
        self.resolve(|node| { sb_table_shape(&node)?;
            let cell=if index<0 { None } else { sb_table_cells(&node).nth(index as usize) };
            let Some(cell)=cell else { return Ok((false,-1,-1,0,0,false)); };
            let (r,c,rs,cs)=sb_cell_shape(&cell)?;Ok((true,r as i32,c as i32,rs as i32,cs as i32,false)) })
    }
    pub fn sb_table_extent_at(&self,row:i32,column:i32,columns:bool)->Result<i32> {
        self.resolve(|node| { sb_table_shape(&node)?;if row<0 || column<0 { return Ok(0); }
            let Some(cell)=sb_table_at(&node,row as usize,column as usize) else { return Ok(0); };
            let (_,_,rs,cs)=sb_cell_shape(&cell)?;Ok(if columns { cs as i32 } else { rs as i32 }) })
    }
    pub fn sb_table_header(&self,index:i32,columns:bool)->Result<Option<FullNodeId>> {
        self.resolve(|node| { let (r,c)=sb_table_shape(&node)?;
            if index<0 || index as usize>=if columns { c } else { r } { return Ok(None); }
            Ok(sb_table_cells(&node).find(|cell|cell.role()==if columns { Role::ColumnHeader } else { Role::RowHeader } &&
                sb_cell_shape(cell).is_ok_and(|(r,c,rs,cs)|if columns { index as usize>=c && (index as usize-c)<cs } else { index as usize>=r && (index as usize-r)<rs })).map(|cell|cell.id())) })
    }
    pub fn sb_table_description(&self,index:i32,columns:bool)->Result<String> {
        self.resolve(|node| { sb_table_shape(&node)?;
            Ok(sb_table_cells(&node).find(|cell|cell.role()==if columns { Role::ColumnHeader } else { Role::RowHeader } &&
                sb_cell_shape(cell).is_ok_and(|(r,c,rs,cs)|index>=0 && if columns { index as usize>=c && (index as usize-c)<cs } else { index as usize>=r && (index as usize-r)<rs }))
                .and_then(|cell|NodeWrapper(&cell).name()).unwrap_or_default()) })
    }
    pub fn sb_cell_extents(&self)->Result<(i32,i32,i32,i32)> { self.resolve(|node|sb_cell_shape(&node).map(|(r,c,rs,cs)|(r as i32,c as i32,rs as i32,cs as i32))) }
    pub fn sb_cell_table(&self)->Result<FullNodeId> { self.resolve(|node| { sb_cell_shape(&node)?;Ok(sb_table_parent(&node).ok_or(Error::UnsupportedInterface)?.id()) }) }
    pub fn sb_cell_headers(&self,columns:bool)->Result<Vec<FullNodeId>> {
        self.resolve(|node| { let (r,c,_,_)=sb_cell_shape(&node)?;let table=sb_table_parent(&node).ok_or(Error::UnsupportedInterface)?;
            Ok(sb_table_cells(&table).filter(|header|header.role()==if columns { Role::ColumnHeader } else { Role::RowHeader } &&
                sb_cell_shape(header).is_ok_and(|(hr,hc,hrs,hcs)|if columns { c>=hc && c-hc<hcs } else { r>=hr && r-hr<hrs })).map(|header|header.id()).collect()) })
    }
}
