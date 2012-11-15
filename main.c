/*
  libachie - backend library for point system for task orientated applications
  Copyright (C) 2012 Eddie Blundell eblundell@gmail.com

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
ac_last_task(struct ac_instance *instance)
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
ac_last_group(struct ac_instance *instance)
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
ac_add_task(struct ac_instance *instance,char *s_task)
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

  struct ac_task *prev = ac_last_task(instance);
  
  if(prev){
    task->prev = prev;
    prev->next = task;

    task->id = prev->id+1;
  }else{
    instance->tasks = task;
  }
}

void
ac_add_group(struct ac_instance *instance, char *s_title,char *s_desc)
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

  struct ac_group *prev = ac_last_group(instance);

  if(prev){
    group->prev = prev;
    prev->next = group;

    group->id = prev->id+1;
  }else{

    instance->groups = group;
  }
  
}
struct ac_task*
ac_task_from_id(struct ac_instance *instance, u_int task_id)
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
ac_group_from_id(struct ac_instance *instance, u_int group_id)
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
ac_count_tasks(struct ac_instance *instance)
{
  if(!instance){
    // Error
    return -1;
  }

  struct ac_task *tmp = instance->tasks;

  if(!tmp){
    return 0;
  }

  int i = 1;
  while((tmp=tmp->next) && ++i)
    ;;

  return i;
}

int 
ac_count_groups(struct ac_instance *instance)
{
  if(!instance){
    // Error
    return -1;
  }

  struct ac_group *tmp = instance->groups;

  if(!tmp){
    return 0;
  }

  int i = 1;
  while((tmp=tmp->next) && ++i)
    ;;

  return i;
}

int
date_diff(u_int timestamp){

  //TODO: Check this with negatives

  u_int today = get_local_time();
  u_int diff = today - timestamp;
  
  return (diff / 86400);
}

void
ac_calculate_points(struct ac_instance *instance)
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
ac_delete_task(struct ac_instance *instance, u_int task_id)
{
  if(!instance){
    // Error
    return;
  }

  struct ac_task *task = ac_task_from_id(instance, task_id);
  
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
ac_free_instance(struct ac_instance *instance)
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
ac_save_instance(struct ac_instance *instance,char *path)
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

  u_int tasks = ac_count_tasks(instance);
  u_int groups = ac_count_groups(instance);

  fwrite(instance->user,sizeof(struct ac_user),1,fp);
  
  fwrite(&tasks, sizeof(u_int),1,fp);
  fwrite(&groups, sizeof(u_int),1,fp);

  fwrite(&instance->multiplier,sizeof(u_int),1,fp);
  fwrite(&instance->threshold,sizeof(u_int),1,fp);

  struct ac_group *g_tmp = instance->groups;

  while(g_tmp){
    fwrite(g_tmp,sizeof(struct ac_group),1,fp);
    g_tmp = g_tmp->next;
  }

  struct ac_task *t_tmp = instance->tasks;

  while(t_tmp){
    fwrite(t_tmp,sizeof(struct ac_task),1,fp);
    t_tmp = t_tmp->next;
  }


  fclose(fp);
}

struct ac_instance*
ac_load_instance(char *path)
{
  FILE *fp;
  fp = fopen(path,"rb");

  if(!fp){
    // Error
    return NULL;
  }

  struct ac_instance* instance;
  instance = malloc(sizeof(struct ac_instance));
  instance->tasks = NULL;
  instance->groups = NULL;

  // Read user info in
  instance->user = malloc(sizeof(struct ac_user));
  fread(instance->user,1,sizeof(struct ac_user),fp);

  u_int groups,tasks;
  groups = tasks = 0;

  fread(&tasks,1,sizeof(u_int),fp);
  fread(&groups,1,sizeof(u_int),fp);
  
  fread(&instance->threshold,1,sizeof(u_int),fp);
  fread(&instance->multiplier,1,sizeof(u_int),fp);
  
  u_int i;
  struct ac_group *g_tmp,*g_prev;
  g_tmp = g_prev = NULL;
  
  for(i = 0; i < groups; i++){
    
    g_prev = g_tmp;

    // TODO: Handle malloc error
    g_tmp = malloc(sizeof(struct ac_group));
    fread(g_tmp,sizeof(struct ac_group),1,fp);

    if(g_prev){
      g_prev->next = g_tmp;
      g_tmp->prev = g_prev;
    }
  }
  
  struct ac_task *t_tmp, *t_prev;
  t_tmp = t_prev = NULL;

  for(i = 0; i < tasks; i++){
    t_prev = t_tmp;

    t_tmp = malloc(sizeof(struct ac_task));
    fread(t_tmp,sizeof(struct ac_task),1,fp);

    if(t_prev){
      t_prev->next = t_tmp;
      t_tmp->prev = t_prev;
    }
  }

  fclose(fp);
}

struct ac_group*
ac_default_group()
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
ac_dump_instance(struct ac_instance *instance)
{
  if(!instance){
    // Error
    return;
  }

  printf("User Points: %d\n",instance->user->points);

  struct ac_group *t_group;
  struct ac_task *t_task;

  t_task = instance->tasks;

  while(t_task){
    
    t_group = ac_group_from_id(instance,t_task->group);
    printf("Task: %s Group: %s \n", t_task->task,t_group->title);
    t_task = t_task->next;
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
  instance->groups = ac_default_group();

  ac_add_group(instance, "Work", "Work group");
  ac_add_task(instance, "Hello World!");
  ac_add_task(instance, "This is a test");
  ac_add_task(instance, "This is a another test");
  ac_add_task(instance, "This is a boom boom shake the room test");

  instance->tasks->complete = 1;
  instance->tasks->group = 1;
  instance->tasks->creation_date = get_local_time();
  instance->tasks->next->complete = 1;

  ac_delete_task(instance,2);

  ac_calculate_points(instance);
  
  ac_save_instance(instance,"test.ac");
  ac_load_instance("test.ac");

  ac_dump_instance(instance);

  ac_free_instance(instance);

  free(user);
  free(instance);


  return 0;
}
