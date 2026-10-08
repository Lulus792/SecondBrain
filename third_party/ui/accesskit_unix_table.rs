// SecondBrain UI integration; MIT, see the repository LICENSE.
use accesskit_atspi_common::PlatformNode;
use zbus::{fdo,interface,names::OwnedUniqueName};
use crate::atspi::{ObjectId,OwnedObjectAddress};

pub(crate) struct TableInterface { bus_name:OwnedUniqueName,node:PlatformNode }
impl TableInterface {
    pub fn new(bus_name:OwnedUniqueName,node:PlatformNode)->Self { Self {bus_name,node} }
    fn map_error(&self)->impl '_+FnOnce(accesskit_atspi_common::Error)->fdo::Error { |error|crate::util::map_error_from_node(&self.node,error) }
    fn object(&self,id:Option<accesskit_atspi_common::FullNodeId>)->OwnedObjectAddress {
        id.map(|node|ObjectId::Node {adapter:self.node.adapter_id(),node}.to_address(&self.bus_name)).unwrap_or_else(OwnedObjectAddress::null)
    }
    fn check(&self)->fdo::Result<()> { self.node.sb_table_dimensions().map(|_|()).map_err(self.map_error()) }
}
#[interface(name="org.a11y.atspi.Table")]
impl TableInterface {
    #[zbus(property,name="version")] fn version(&self)->u32 { 1 }
    #[zbus(property)] fn n_rows(&self)->fdo::Result<i32> { self.node.sb_table_dimensions().map(|(r,_)|r).map_err(self.map_error()) }
    #[zbus(property)] fn n_columns(&self)->fdo::Result<i32> { self.node.sb_table_dimensions().map(|(_,c)|c).map_err(self.map_error()) }
    #[zbus(property)] fn caption(&self)->fdo::Result<OwnedObjectAddress> { self.check()?;Ok(OwnedObjectAddress::null()) }
    #[zbus(property)] fn summary(&self)->fdo::Result<OwnedObjectAddress> { self.check()?;Ok(OwnedObjectAddress::null()) }
    #[zbus(property)] fn n_selected_rows(&self)->fdo::Result<i32> { self.check()?;Ok(0) }
    #[zbus(property)] fn n_selected_columns(&self)->fdo::Result<i32> { self.check()?;Ok(0) }
    fn get_accessible_at(&self,row:i32,column:i32)->fdo::Result<(OwnedObjectAddress,)> { Ok((self.object(self.node.sb_table_cell_at(row,column).map_err(self.map_error())?),)) }
    fn get_index_at(&self,row:i32,column:i32)->fdo::Result<i32> { self.node.sb_table_index_at(row,column).map_err(self.map_error()) }
    fn get_row_at_index(&self,index:i32)->fdo::Result<i32> { self.node.sb_table_extents_at_index(index).map(|(_,r,_,_,_,_)|r).map_err(self.map_error()) }
    fn get_column_at_index(&self,index:i32)->fdo::Result<i32> { self.node.sb_table_extents_at_index(index).map(|(_,_,c,_,_,_)|c).map_err(self.map_error()) }
    fn get_row_extent_at(&self,row:i32,column:i32)->fdo::Result<i32> { self.node.sb_table_extent_at(row,column,false).map_err(self.map_error()) }
    fn get_column_extent_at(&self,row:i32,column:i32)->fdo::Result<i32> { self.node.sb_table_extent_at(row,column,true).map_err(self.map_error()) }
    fn get_row_header(&self,row:i32)->fdo::Result<(OwnedObjectAddress,)> { Ok((self.object(self.node.sb_table_header(row,false).map_err(self.map_error())?),)) }
    fn get_column_header(&self,column:i32)->fdo::Result<(OwnedObjectAddress,)> { Ok((self.object(self.node.sb_table_header(column,true).map_err(self.map_error())?),)) }
    fn get_row_description(&self,row:i32)->fdo::Result<String> { self.node.sb_table_description(row,false).map_err(self.map_error()) }
    fn get_column_description(&self,column:i32)->fdo::Result<String> { self.node.sb_table_description(column,true).map_err(self.map_error()) }
    fn get_selected_rows(&self)->fdo::Result<Vec<i32>> { self.check()?;Ok(Vec::new()) }
    fn get_selected_columns(&self)->fdo::Result<Vec<i32>> { self.check()?;Ok(Vec::new()) }
    fn is_row_selected(&self,_row:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn is_column_selected(&self,_column:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn is_selected(&self,_row:i32,_column:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn add_row_selection(&self,_row:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn add_column_selection(&self,_column:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn remove_row_selection(&self,_row:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn remove_column_selection(&self,_column:i32)->fdo::Result<bool> { self.check()?;Ok(false) }
    fn get_row_column_extents_at_index(&self,index:i32)->fdo::Result<(bool,i32,i32,i32,i32,bool)> { self.node.sb_table_extents_at_index(index).map_err(self.map_error()) }
}

pub(crate) struct TableCellInterface { bus_name:OwnedUniqueName,node:PlatformNode }
impl TableCellInterface {
    pub fn new(bus_name:OwnedUniqueName,node:PlatformNode)->Self { Self {bus_name,node} }
    fn map_error(&self)->impl '_+FnOnce(accesskit_atspi_common::Error)->fdo::Error { |error|crate::util::map_error_from_node(&self.node,error) }
    fn object(&self,node:accesskit_atspi_common::FullNodeId)->OwnedObjectAddress { ObjectId::Node {adapter:self.node.adapter_id(),node}.to_address(&self.bus_name) }
    fn headers(&self,columns:bool)->fdo::Result<Vec<OwnedObjectAddress>> { Ok(self.node.sb_cell_headers(columns).map_err(self.map_error())?.into_iter().map(|id|self.object(id)).collect()) }
}
#[interface(name="org.a11y.atspi.TableCell")]
impl TableCellInterface {
    #[zbus(property,name="version")] fn version(&self)->u32 { 1 }
    #[zbus(property)] fn position(&self)->fdo::Result<(i32,i32)> { self.node.sb_cell_extents().map(|(r,c,_,_)|(r,c)).map_err(self.map_error()) }
    #[zbus(property)] fn row_span(&self)->fdo::Result<i32> { self.node.sb_cell_extents().map(|(_,_,r,_)|r).map_err(self.map_error()) }
    #[zbus(property)] fn column_span(&self)->fdo::Result<i32> { self.node.sb_cell_extents().map(|(_,_,_,c)|c).map_err(self.map_error()) }
    #[zbus(property)] fn table(&self)->fdo::Result<OwnedObjectAddress> { Ok(self.object(self.node.sb_cell_table().map_err(self.map_error())?)) }
    fn get_row_column_span(&self)->fdo::Result<(bool,i32,i32,i32,i32)> { self.node.sb_cell_extents().map(|(r,c,rs,cs)|(true,r,c,rs,cs)).map_err(self.map_error()) }
    fn get_column_header_cells(&self)->fdo::Result<Vec<OwnedObjectAddress>> { self.headers(true) }
    fn get_row_header_cells(&self)->fdo::Result<Vec<OwnedObjectAddress>> { self.headers(false) }
}
