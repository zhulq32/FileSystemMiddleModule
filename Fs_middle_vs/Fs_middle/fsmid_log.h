#ifndef _FSMID_LOG_H_
#define _FSMID_LOG_H_

#include "fsmid_type.h"

/**
 * fsmid_register() - register log function
 * @fileHandle: handle of file system
 * @arrayFunction: FSPORT_FUNCTION array registered to file
 * @groupCount: number of log group in the file
 *
 * Return: error code, 0 on success
 *
 * This function initialize log group
 */
int fsmid_register(FSMID_FHANDLE fileHandle, const FSPORT_FUNCTION *arrayFunction, unsigned int groupCount);

/**
 * fsmid_push() - add log handle into file
 * @fileHandle: handle of file system
 * @logHandle: handle of log component
 * @groupIndex: index of log package
 *
 * Return: error code, 0 on success
 *
 * This function add log into file->list, and will call get_length()
 * to update file->length, call get_time() to update file->time.
 */
int fsmid_push(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle, unsigned int groupIndex);

/**
 * fsmid_pop() - get time of log data
 * @fileHandle: handle of file system
 * @logHandle: handle of log component
 *
 * Return: error code, 0 on success
 *
 * This function remove log from file->list, and will call get_length()
 * to update file->length, call get_time() to update file->time.
 */
int fsmid_pop(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle);

#endif
