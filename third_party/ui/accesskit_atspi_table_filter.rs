// SecondBrain UI integration; MIT, see the repository LICENSE.
// AT-SPI table indices refer to direct accessible children. Make row wrappers
// transparent so matrix indices, parents and cache items share the same tree.
pub(crate) fn filter(node: &accesskit_consumer::NodeRef) -> accesskit_consumer::FilterResult {
    use accesskit_consumer::{common_filter,FilterResult};
    let result=common_filter(node);
    if result==FilterResult::Include && node.role()==accesskit::Role::Row && node.parent().is_some_and(|parent|
        matches!(parent.role(),accesskit::Role::Table | accesskit::Role::Grid | accesskit::Role::TreeGrid) &&
        parent.data().row_count().is_some() && parent.data().column_count().is_some()) { FilterResult::ExcludeNode } else { result }
}
