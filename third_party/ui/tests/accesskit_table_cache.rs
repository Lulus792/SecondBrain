// SecondBrain UI adapter regression test; MIT, see the repository LICENSE.
use accesskit::{ActionHandler,ActionRequest,Node,NodeId,Role,TreeInfo,TreeId,TreeUpdate};
use crate::{Adapter,AdapterCallback,AppContext,Event,FullNodeId,InterfaceSet,WindowBounds};
use std::{sync::{Arc,Mutex},time::Instant};
struct Callback(Arc<Mutex<Vec<FullNodeId>>>);
impl AdapterCallback for Callback {
 fn register_interfaces(&self,_:&Adapter,id:FullNodeId,_:InterfaceSet){self.0.lock().unwrap().push(id);}
 fn unregister_interfaces(&self,_:&Adapter,_:FullNodeId,_:InterfaceSet){}
 fn emit_event(&self,_:&Adapter,_:Event){}
}
struct Actions;
impl ActionHandler for Actions {fn do_action(&mut self,_:ActionRequest){}}
#[test]
fn rectangular_indices_and_cache_preserve_tree_identity(){
 let rows=512;let columns=32;let mut root=Node::new(Role::Window);root.set_children(vec![NodeId(1)]);
 let mut table=Node::new(Role::Table);table.set_author_id("reader:table:42");table.set_row_count(rows);table.set_column_count(columns);
 table.set_children((0..rows).map(|r|NodeId((2+r) as u64)).collect::<Vec<_>>());
 let mut nodes=vec![(NodeId(0),root),(NodeId(1),table)];
 for r in 0..rows {
  let mut row=Node::new(Role::Row);row.set_row_index(r);
  row.set_children((0..columns).map(|c|NodeId((2+rows+r*columns+c) as u64)).collect::<Vec<_>>());
  nodes.push((NodeId((2+r) as u64),row));
  for c in 0..columns {let mut cell=Node::new(if r==0 {Role::ColumnHeader}else{Role::Cell});cell.set_row_index(r);cell.set_column_index(c);cell.set_row_span(1);cell.set_column_span(1);nodes.push((NodeId((2+rows+r*columns+c) as u64),cell));}
 }
 let ids=Arc::new(Mutex::new(Vec::new()));let context=AppContext::new(None);
 let start=Instant::now();let mut adapter=Adapter::new(&context,Callback(ids.clone()),TreeUpdate{nodes,tree:Some(TreeInfo::new(NodeId(0))),tree_id:TreeId::ROOT,focus:NodeId(0)},false,WindowBounds::default(),Actions);
 println!("Adapter build {:?}; registered {}",start.elapsed(),ids.lock().unwrap().len());
 let cells=ids.lock().unwrap().clone();let start=Instant::now();let mut count=0;
 for (index,id) in cells.iter().skip(2).enumerate(){let node=adapter.platform_node(*id);let cache=node.cache_node().unwrap();assert_eq!(cache.index_in_parent,index as i32);assert_eq!(node.index_in_parent().unwrap(),index as i32);count+=1;}
 println!("{} cache and index queries in {:?}; {} cells",count,start.elapsed(),rows*columns);
 let table=adapter.platform_node(cells[1]);assert_eq!(table.child_count().unwrap(),(rows*columns) as i32);
 for index in [0,columns-1,rows*columns/2,rows*columns-1]{assert_eq!(table.child_at_index(index).unwrap(),Some(cells[index+2]));}
 // Incorrect row metadata must use actual sibling order, not invent an index.
 let mut wrong=Node::new(Role::ColumnHeader);wrong.set_row_index(2);wrong.set_column_index(0);wrong.set_row_span(1);wrong.set_column_span(1);
 adapter.update(TreeUpdate{nodes:vec![(NodeId((2+rows) as u64),wrong)],tree:None,tree_id:TreeId::ROOT,focus:NodeId(0)});
 assert_eq!(adapter.platform_node(cells[2]).cache_node().unwrap().index_in_parent,0);
 assert_eq!(table.child_at_index(0).unwrap(),Some(cells[2]));
 let stale=adapter.platform_node(cells[2]);let root=Node::new(Role::Window);
 adapter.update(TreeUpdate{nodes:vec![(NodeId(0),root)],tree:None,tree_id:TreeId::ROOT,focus:NodeId(0)});
 assert!(stale.cache_node().is_err());assert!(stale.index_in_parent().is_err());
 println!("All 16384 indices, independent child lookup, malformed metadata fallback and defunct context passed.");
}
