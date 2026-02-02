#ifndef INTELLISENSE_MACRO_H
#define INTELLISENSE_MACRO_H
// VScode has no clue what this compiler thing is and marks entire file as error
// here we are telling it it's just nothing lol
#ifdef __INTELLISENSE__
#define fastcall 
#define __fastcall__ 
#endif


#endif