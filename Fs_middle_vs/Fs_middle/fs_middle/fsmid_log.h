#ifndef _FSMID_LOG_H_
#define _FSMID_LOG_H_

#include "fsmid_type.h"

/**
 * FSMID_Register() - register log function
 * @fileHandle: handle of file system
 * @arrayFunction: FSPORT_FUNCTION array registered to file
 * @groupCount: number of log group in the file
 *
 * Return: error code, 0 on success
 *
 * This function initialize log group
 */
int FSMID_Register(FSMID_FHANDLE fileHandle, const FSPORT_FUNCTION *arrayFunction, unsigned int groupCount);

/**
 * FSMID_Push() - add log handle into file
 * @fileHandle: handle of file system
 * @logHandle: handle of log component
 * @groupIndex: index of log package
 *
 * Return: error code, 0 on success
 *
 * This function add log into file->list, and will call get_length()
 * to update file->length, call get_time() to update file->time.
 */
int FSMID_Push(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle, unsigned int groupIndex);

/**
 * FSMID_Pop() - get time of log data
 * @fileHandle: handle of file system
 * @logHandle: handle of log component
 *
 * Return: error code, 0 on success
 *
 * This function remove log from file->list, and will call get_length()
 * to update file->length, call get_time() to update file->time.
 */
int FSMID_Pop(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle);

#endif