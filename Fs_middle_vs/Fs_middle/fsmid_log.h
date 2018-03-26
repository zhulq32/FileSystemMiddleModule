#ifndef _FSMID_LOG_H_
#define _FSMID_LOG_H_

#include "fsmid_port.h"
#include "fsmid_def.h"

/**
 * fsmid_push() - add log handle into file
 * @fileHandle: handle of file system
 * @logHandle: handle of log component
 *
 * Return: error code, 0 on success
 *
 * This function add log into file->list, and will call fsport_length()
 * to update file->length, call fsport_time() to update file->time.
 */
int fsmid_push(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle);

/**
 * fsmid_pop() - get time of log data
 * @fileHandle: handle of file system
 * @logHandle: handle of log component
 *
 * Return: error code, 0 on success
 *
 * This function remove log from file->list, and will call fsport_length()
 * to update file->length, call fsport_time() to update file->time.
 */
int fsmid_pop(FSMID_FHANDLE handle, FSMID_LOG_HANDLE logHandle);

#endif
