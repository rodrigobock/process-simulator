#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <limits.h>

#define MAX_NAME_LENGTH 50

enum ProcessStatus {
  RUNNING,
  READY,
  STOPPED
};

enum ProcessPriority{
  HIGH,
  MEDIUM,
  LOW
};

struct Registers {
  unsigned short EAX;
  unsigned short EBX;
  unsigned short ECX;
  unsigned short EDX;
  unsigned short ESI;
  unsigned short EDI;
  unsigned short EBP;
  unsigned short ESP;
};

typedef struct Process {
  int pid;
  char name[MAX_NAME_LENGTH];
  int priority;
  int status;
  struct Registers registers;
  unsigned int start_time;
  unsigned int end_time;
  struct Process *next;
} Process;

void generate_random_registers(struct Registers *registers) {
  registers->EAX = rand() % USHRT_MAX;
  registers->EBX = rand() % USHRT_MAX;
  registers->ECX = rand() % USHRT_MAX;
  registers->EDX = rand() % USHRT_MAX;
  registers->ESI = rand() % USHRT_MAX;
  registers->EDI = rand() % USHRT_MAX;
  registers->EBP = rand() % USHRT_MAX;
  registers->ESP = rand() % USHRT_MAX;
}

Process *create_process(char *name, int priority) {
  
  Process *process = malloc(sizeof(Process));
  
  process->pid = 1;
  strcpy(process->name, name);
  process->priority = priority;
  process->status = STOPPED;
  generate_random_registers(&(process->registers));
  process->start_time = 0;
  process->end_time = 0;
  process->next = NULL;

  return process;

}

Process *fork_process(Process *parent_process, char *name, int priority, int pid) {
  
  Process *child_process = create_process(name, priority);

  child_process->pid = pid;
  child_process->status = parent_process->status;
  generate_random_registers(&(child_process->registers));

  if (parent_process->next == NULL) {
    parent_process->next = child_process;
  } else {
    Process *current = parent_process->next;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = child_process;
  }

  return child_process;

}

void list_process_infos(Process *process) {
  printf("\n");

  printf("Process id: %d\n", process->pid);
  printf("Process name: %s\n", process->name);
  printf("Process priority: %d\n", process->priority);
  printf("Process status: %d\n", process->status);
  printf("Process EAX: %hu\n", process->registers.EAX);
  printf("Process EBX: %hu\n", process->registers.EBX);
  printf("Process ECX: %hu\n", process->registers.ECX);
  printf("Process EDX: %hu\n", process->registers.EDX);
  printf("Process ESI: %hu\n", process->registers.ESI);
  printf("Process EDI: %hu\n", process->registers.EDI);
  printf("Process EBP: %hu\n", process->registers.EBP);
  printf("Process ESP: %hu\n", process->registers.ESP);

  if (process->next != NULL) {
    printf("Next Process: %d\n", process->next->pid);
  } else {
    printf("Next Process: None\n");
  }

  if (process->end_time != 0 && process->start_time != 0) {
    unsigned int execution_time = process->end_time - process->start_time;
    printf("Execution time: %u seconds\n", execution_time);
  } else {
    printf("Execution time: %d seconds\n", 0);
  }

  printf("\n");
}

void run_process(Process *process) {
  if (process->status == READY) {
    
    if (process->start_time == 0) {
      process->start_time = time(NULL);
    }

    process->status = RUNNING;
  
    printf("Executing process: %s\n", process->name);
    printf("Process status: RUNNING\n");

    if (process->priority == HIGH) {
      sleep(3);
    } else if (process->priority == MEDIUM) {
      sleep(2);
    } else {
      sleep(1);
    }
    
    
    printf("\n");

    process->status = STOPPED;
    process->end_time = time(NULL);
    printf("Process execution finished: %s\n", process->name);
    printf("Process status: STOPPED\n");
    printf("\n");
  }
}

Process* find_process_by_pid(Process* head, int pid) {

  Process* current = head;

  while (current != NULL) {
    if (current->pid == pid) {
      return current;
    }
    current = current->next;
  }

  return NULL;

}

void delete_process_by_pid(Process** head, int pid) {
  
  if (*head == NULL) {
    return;
  }

  Process* current = *head;
  Process* previous = NULL;

  // Se o processo a ser excluido é o primeiro da lista,
  // remove e o processo filho vira o processo pai
  if (current != NULL && current->pid == pid) {
    *head = current->next;
    free(current);
    return;
  }

  // Percorre a lista em busca do processo a ser excluído
  while (current != NULL && current->pid != pid) {
    previous = current;
    current = current->next;
  }

  // Se o processo foi encontrado, o remove da lista
  if (current != NULL) {
    previous->next = current->next;
    free(current);
  }

}

void delete_all_child_process_by_pid(Process **process_list, int pid) {
  
  if (*process_list == NULL) {
    return;  // Lista vazia, nenhum processo para excluir
  }

  Process *current = *process_list;
  Process *previous = NULL;

  while (current != NULL) {

    if (current->pid == pid) {
      // Encontrou o processo a ser excluído

      if (previous != NULL) {
        // Atualizar o ponteiro do nó anterior para o próximo nó
        previous->next = current->next;
      } else {
        // O processo a ser excluído é o primeiro da lista
        *process_list = current->next;
      }

      free(current);  // Liberar memória do processo excluído

      // Verificar se o próximo nó é um processo filho e excluí-lo recursivamente
      if (current->next != NULL && current->next->pid != pid) {
        delete_all_child_process_by_pid(process_list, current->next->pid);
      }

      current = current->next; 
    } else {
      previous = current;
      current = current->next;
    }

  }

}


int kbhit() {
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}

void list_all_processes(Process* head) {
  Process* current = head;

  while (current != NULL) {
    list_process_infos(current);
    current = current->next;
  }
}


int main() {
  srand(time(NULL));

  Process *parent_process = create_process("Parent", HIGH);
  Process *child1_process = fork_process(parent_process, "Child 1", MEDIUM, 123);
  Process *child2_process = fork_process(parent_process, "Child 2", LOW, 321);

  // Process* found_process = find_process_by_pid(parent_process, 123);
  // list_process_infos(found_process);
  // delete_all_child_process_by_pid(&parent_process, 123);

  printf("Executing processes...\n");

  char ch;
  int isRunning = 1;

  printf("Pressione a tecla 'q' para interromper a execução.\n");

  // Configurar terminal para leitura não bloqueante
  struct termios old_tio, new_tio;
  tcgetattr(STDIN_FILENO, &old_tio);
  new_tio = old_tio;
  new_tio.c_lflag &= (~ICANON & ~ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  srand(time(NULL));

  while (isRunning) {
    
    double randomNum = drand48();  

    if(randomNum <= 0.5){

      if(parent_process->status == STOPPED){
        parent_process->status = READY;
      } else if (parent_process->status){
        run_process(parent_process);
      }

    } else if (randomNum > 0.5 && randomNum <= 0.8){

      if(child1_process->status == STOPPED){
        child1_process->status = READY;
      } else if (child1_process->status){
        run_process(child1_process);
      }

    } else if (randomNum > 0.8){

      if(child2_process->status == STOPPED){
        child2_process->status = READY;
      } else if (child2_process->status){
        run_process(child2_process);
      }
    }

    // Verificar se a tecla 'q' foi pressionada
    if (kbhit()) {
      ch = getchar();
      if (ch == 'q') {
        printf("Tecla 'q' pressionada. Interrompendo a execução...\n");
        list_all_processes(parent_process);
        isRunning = 0;
      }
    }

  }

  // Restaurar configurações do terminal
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

  return 0;
}
