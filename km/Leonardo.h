#ifndef LEONARDO_H 
#define LEONARDO_H 

#define MUTEX _IOW(0, 6, struct mutex_struct) 

#define CONDITION _IOW(0, 6, struct condition_struct) 


struct mutex_struct
{ 
  int pid;
  int id; 
  int action; 
  int test;
};

// struct for conditions. REMEMBER NO TYPEDEF HERE SO YOU HAVE TO USE WORD "STRUCT" WHEN DECLARING IT
struct condition_struct 
{ 
  int id; 
  int action; 
  int mutex_id;
  struct mutex_struct rafael;
  struct mutex_struct sleeper; // why not
};

#endif
