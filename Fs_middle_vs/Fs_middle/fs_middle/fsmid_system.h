#ifndef _FSMID_SYSTEM_H_
#define _FSMID_SYSTEM_H_


void FSMID_Init(const struct __fsmid_callback *pCallback);

FSMID_FILE * fsmid_search( const char* pPathName);

int fsmid_set_path_name( FSMID_FILE *pFile, const char *pPathName);

void fsmid_get_path_name( FSMID_FILE *pFile, char *pPathName);

int fsmid_create_new(FSMID_FILE **ppFile, const char *pPathName, FSMID_OPEN_ATTR attribute);

int fsmid_init_exist(FSMID_FILE *pFile);

int fsmid_ulink_system(FSMID_FILE* pFile);

void fsmid_release_close(FSMID_FILE *pFile);

//int fsmid_get_element_data(FSMID_FILE *pFile, FSMID_ELEMENT *element);

FSMID_ELEMENT* fsmid_next_element(FSMID_FILE *pFile);

FSMID_ELEMENT* fsmid_prev_element(FSMID_FILE *pFile);

int fsmid_search_log(FSMID_FILE *pFile, unsigned int offset);

#endif
