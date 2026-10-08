#ifndef SB_TEXT_CARETS_H
#define SB_TEXT_CARETS_H
#include "shaped.h"
/* Affinity names the adjacent logical grapheme. A bidi boundary can have
   different visual positions for the same source byte. BOTH merges them only
   when their position and embedding level agree. */
typedef enum {SB_CARET_BEFORE=1,SB_CARET_AFTER=2,SB_CARET_BOTH=3} SBCaretAffinity;
typedef struct {size_t byte;float x;unsigned char level,affinity;} SBCaret;
typedef enum {SB_CARET_ADVANCE,SB_CARET_GDEF,SB_CARET_PROPORTIONAL} SBCaretMetric;
typedef struct {size_t byte,length;float left,right;unsigned char level,metric;} SBCaretCluster;
typedef struct {size_t byte,stop;} SBCaretIndex;
typedef struct {
    SBCaretIndex *logical;
    SBCaret *stops;size_t count,capacity;
    SBCaretCluster *clusters;size_t cluster_count,cluster_capacity;
    size_t byte,length;
} SBCaretPlan;
typedef struct {float x,width;size_t byte,length;unsigned char level;} SBSelectionSpan;
typedef struct {SBSelectionSpan *spans;size_t count;} SBSelectionPlan;
/* Uses the exact shaped line and its immutable paragraph. Backing pixels,
   logical UTF-8 offsets, full grapheme boundaries only. Output is empty.
   GDEF ligature positions are preferred; otherwise proportional grapheme
   positions within the shaped cluster are a documented approximation. */
SBStatus sb_caret_plan(const SBShapeParagraph *paragraph,const SBShapedLine *line,SBCaretPlan *out);
void sb_caret_plan_free(SBCaretPlan *plan);
/* Resolve an explicit affinity, or use the paragraph direction for a default.
   nearest clamps at the visual edges and has a deterministic tie rule. */
bool sb_caret_find(const SBCaretPlan *plan,size_t byte,SBCaretAffinity affinity,unsigned char base_level,SBCaret *out);
bool sb_caret_nearest(const SBCaretPlan *plan,float x,unsigned char base_level,SBCaret *out);
bool sb_caret_step(const SBCaretPlan *plan,const SBCaret *from,int direction,SBCaret *out);
/* Logical selection becomes possibly disconnected visual rectangles. The
   end points must be grapheme boundaries; no source byte is changed. */
SBStatus sb_caret_selection(const SBCaretPlan *plan,size_t begin,size_t end,SBSelectionPlan *out);
void sb_caret_selection_free(SBSelectionPlan *plan);
#endif
