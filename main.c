#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef u_int u_int;

struct ac_user{

  int achievments[255];
  int points;

};


struct ac_group {

  u_int id;
  char title[255];
  char description[255];
  struct ac_group *next;
  struct ac_group *prev;
};

struct ac_task {

  u_int id;
  u_int creation_date;
  u_int priority;
  u_int tag;
  u_int group;
  u_int complete;
  u_int processed;

  u_int ongoing;

  char task[255];

  struct ac_task *prev;
  struct ac_task *next;
};

struct ac_instance{

  u_int multiplier;
  u_int threshold;
  struct ac_user  *user;
  struct ac_group *groups;
  struct ac_task  *tasks;
};

// Standard achievment 
struct ac_achievment{

  char *title[255];
  u_int points;
  u_int days;
};

int 
get_local_time()
{
  time_t tm;
  time(&tm);
  return tm;
}

struct ac_task*
get_last_task(struct ac_instance *instance)
{
  if(!instance){
    // die
    return NULL;
  }

  struct ac_task* tmp;
  tmp = instance->tasks;
  
  while(tmp){
  
    if(!tmp->next){
      return tmp;
    }

    tmp = tmp->next;
  }

  return NULL;
}

struct ac_group*
get_last_group(struct ac_instance *instance)
{
  if(!instance){
    // Error
    return NULL;
  }

  struct ac_group *tmp;
  tmp = instance->groups;

  while(tmp){
    if(!tmp->next){
      return tmp;
    }

    tmp = tmp->next;
  }
  return NULL;
}   

void 
add_task(struct ac_instance *instance,char *s_task)
{
  if(!instance){
    // Error
    return;
  }

  struct ac_task *task;
  task = malloc(sizeof(struct ac_task));

  task->creation_date = get_local_time();
  task->priority = 0;
  task->id = 0;
  task->tag = 0;
  task->group = 0;
  task->next = NULL;
  task->prev = NULL;
  task->ongoing = 0;
  task->processed = 0;
  task->complete = 0;

  strcpy(task->task,s_task);

  struct ac_task *prev = get_last_task(instance);
  
  if(prev){
    task->prev = prev;
    prev->next = task;

    task->id = prev->id+1;
  }else{
    instance->tasks = task;
  }
}

void
add_group(struct ac_instance *instance, char *s_title,char *s_desc)
{
  if(!instance){
    // Error
    return;
  }

  struct ac_group *group;
  group = malloc(sizeof(struct ac_group));

  group->id = 0;

  strcpy(group->title,s_title);
  strcpy(group->description,s_desc);

  struct ac_group *prev = get_last_group(instance);

  if(prev){
    group->prev = prev;
    prev->next = group;

    group->id = prev->id+1;
  }else{

    instance->groups = group;
  }
  
}

struct ac_task*
task_from_id(struct ac_instance *instance, u_int task_id)
{
  
  if(!instance){
    // Error
    return NULL;
  }

  struct ac_task *tmp = instance->tasks;
  
  while(tmp){

    if(tmp->id == task_id){
      return tmp;
    }

    tmp = tmp->next;
  }

  return NULL;
}

struct ac_group* 
group_from_id(struct ac_instance *instance, u_int group_id)
{
  if(!instance){
    // Error
    return NULL;
  }

  struct ac_group *tmp = instance->groups;

  while(tmp){
    if(tmp->id == group_id){
      return tmp;
    }
    tmp = tmp->next;
  }

  return NULL;
}


int
date_diff(u_int timestamp){

  //TODO: Check this with negatives

  u_int today = get_local_time();
  u_int diff = today - timestamp;
  
  return (diff / 86400);
}

void
calculate_points(struct ac_instance *instance)
{

  struct ac_task *tmp = instance->tasks;
  int d_diff;

  while(tmp){
  
    if(tmp->ongoing || tmp->processed){
      goto next;
    }

    d_diff = date_diff(tmp->creation_date);
    
    // Not complete and past it's threshold
    if(!tmp->complete && 
        (d_diff >= instance->threshold) ){
      
      instance->user->points -= (instance->threshold - d_diff) * instance->multiplier;
    }

    // Complete within it's threshold
    if(tmp->complete && 
        (d_diff <= instance->threshold) ){
  
      if(!d_diff){
        d_diff = 1;
      }
      instance->user->points += d_diff * instance->multiplier;
    }

    tmp->processed = 1;

next:
    tmp = tmp->next;
  }
}

void 
delete_task(struct ac_instance *instance, u_int task_id)
{
  if(!instance){
    // Error
    return;
  }

  struct ac_task *task = task_from_id(instance, task_id);
  
  if(!task){
    // Error
    return;
  }

  // Start of list?
  if(!task->prev){
    instance->tasks = task->next;
  }else{
    task->prev->next = task->next;
  }

  if(!task->next){
    task->prev->next = NULL;
  }else{
    task->next->prev = task->prev;
  }

  free(task);
}

    
void
free_ac_instance(struct ac_instance *instance)
{
  if(instance->groups){
    struct ac_group *g_tmp = NULL;

    g_tmp = instance->groups;

    while(instance->groups){
      
      g_tmp = instance->groups->next;
      free(instance->groups);
      instance->groups = g_tmp;

    }
  }
  
  instance->groups = NULL;

  if(instance->tasks){
    
    struct ac_task *t_tmp = NULL;

    t_tmp = instance->tasks;
    
    while(instance->tasks){
      
      t_tmp = instance->tasks->next;
      free(instance->tasks);
      instance->tasks = t_tmp;
    }
  }

  instance->tasks = NULL;
}

void 
save_ac_instance(struct ac_instance *instance,char *path)
{
  if(!instance){
    // Error
    return;
  }

  FILE *fp;
  fp = fopen(path,"wb");

  if(!fp){
    // Error
    return;
  }
  


  fclose(fp);
}

struct ac_group*
get_default_group()
{
  struct ac_group *tmp;
  tmp = malloc(sizeof(struct ac_group));

  strcpy(tmp->title,"Default Tasks");
  strcpy(tmp->description,"Default tasks");

  tmp->next = NULL;
  tmp->prev = NULL;

  return tmp;
}

void 
dump_instance(struct ac_instance *instance)
{
  if(!instance){
    // Error
    return;
  }

  printf("User Points: %d\n",instance->user->points);

  struct ac_group *t_group;

  // Dump out tasks
  while(instance->tasks){

    t_group = group_from_id(instance,instance->tasks->group);

    printf("Task: %s Group: %s \n", instance->tasks->task,t_group->title);
    instance->tasks = instance->tasks->next;
  }

}

/*
 * Load Tasks
 * Save Tasks
 * Add Task 
 * Add Group
 * Mark Complete - Should add correct points
 * Check Completions - Should check the date against all tasks and take points if needed
 * Add Achievment 
 *
 * Achievments should be kept seperate from the backend library
 * and referenced with an ID
 *
 *
 */

int main()
{
  struct ac_user *user = malloc(sizeof(struct ac_user));  

  struct ac_instance *instance;
  instance = malloc(sizeof(struct ac_instance));

  instance->multiplier = 10;
  instance->threshold = 3;
  instance->user = user;
  instance->tasks = NULL;
  instance->groups = get_default_group();

  add_group(instance, "Work", "Work group");
  add_task(instance, "Hello World!");
  add_task(instance, "This is a test");
  add_task(instance, "This is a another test");
  add_task(instance, "This is a boom boom shake the room test");

  instance->tasks->complete = 1;
  instance->tasks->group = 1;
  instance->tasks->creation_date = get_local_time();
  instance->tasks->next->complete = 1;

  delete_task(instance,2);

  calculate_points(instance);

  dump_instance(instance);

  free_ac_instance(instance);

  free(user);
  free(instance);


  return 0;
}
