#ifndef _FSMID_SYSTEM_H_
#define _FSMID_SYSTEM_H_

void fsmid_init();

FSMID_FILE * fsmid_search( const char* pPathName);

int fsmid_set_path_name( FSMID_FILE *pFile, const char *pPathName);

FSMID_FILE* fsmid_create_new(const char *pPathName, FSMID_OPEN_ATTR attribute);

FSMID_FILE* fsmid_open_exist(const char *pPathName, FSMID_OPEN_ATTR attribute);

int fsmid_ulink_system(FSMID_FILE* pFile);

int fsmid_release_resource(FSMID_FILE *pFile);

int fsmid_search_log(FSMID_FILE *pFile, unsigned int offset);

FSMID_LIST_LOG* fsmid_next_log(FSMID_FILE *pFile);

FSMID_LIST_LOG* fsmid_prev_log(FSMID_FILE *pFile);

#endif
