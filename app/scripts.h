#ifndef SB_UI_SCRIPTS_H
#define SB_UI_SCRIPTS_H
#include "sb.h"
#define SB_SCRIPT_COMMON UINT32_C(0x5a797979)
#define SB_SCRIPT_INHERITED UINT32_C(0x5a696e68)
#define SB_SCRIPT_UNKNOWN UINT32_C(0x5a7a7a7a)
typedef struct {uint32_t primary,paired;const uint32_t *extensions;size_t count;unsigned char bracket;} SBScriptProperty;
typedef struct {size_t byte,length;uint32_t script;} SBScriptSpan;
typedef struct {SBScriptSpan *spans;size_t count;} SBScriptPlan;
SBScriptProperty sb_script_property(uint32_t codepoint);
/* Own rendering policy based on UAX #24: whole graphemes, contextual Common/
   Inherited, extension compatibility and matching enclosing brackets.
   Output is initially empty; borrowed property arrays are static. */
SBStatus sb_script_plan(const char *text,size_t length,SBScriptPlan *out);
void sb_script_plan_free(SBScriptPlan *plan);
#endif
