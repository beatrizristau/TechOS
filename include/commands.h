#ifndef COMMANDS_H
#define COMMANDS_H

#define TECHOS_VERSION "5.0"
#define AUTHORS "Beatriz Ristau & Tommi Kenneda"
#define COMPLETION_DATE "Dec 7, 2021"

void handle_help(int argc, char *argv[]);
void handle_version(int argc, char *argv[]);
void handle_show_date(int argc, char *argv[]);
void handle_set_date(int argc, char *argv[]);
void handle_show_time(int argc, char *argv[]);
void handle_terminate(int argc, char *argv[]);
void handle_create_pcb(int argc, char *argv[]);
void handle_show_all_pcbs(int argc, char *argv[]);
void handle_delete_pcb(int argc, char *argv[]);
void handle_block_pcb(int argc, char *argv[]);
void handle_unblock_pcb(int argc, char *argv[]);
void handle_suspend_pcb(int argc, char *argv[]);
void handle_resume_pcb(int argc, char *argv[]);
void handle_set_pcb_priority(int argc, char *argv[]);
void handle_show_pcb(int argc, char *argv[]);
void handle_show_ready_pcbs(int argc, char *argv[]);
void handle_show_blocked_pcbs(int argc, char *argv[]);
void handle_load_pcbs(int argc, char *argv[]);
void handle_dispatch_pcbs(int argc, char *argv[]);
void handle_clear(int argc, char *argv[]);
void handle_view_directory(int argc, char *argv[]);
void handle_change_directory(int argc, char *argv[]);
void handle_create_directory(int argc, char *argv[]);
void handle_remove_directory(int argc, char *argv[]);
void handle_create_file(int argc, char *argv[]);
void handle_remove_file(int argc, char *argv[]);
void handle_add_user(int argc, char *argv[]);
void handle_remove_user(int argc, char *argv[]);
void handle_change_password(int argc, char *argv[]);
void handle_add_admin(int argc, char *argv[]);
void handle_remove_admin(int argc, char *argv[]);

#endif
