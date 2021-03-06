#ifndef TRACE_FILE_H
#define TRACE_FILE_H

/*
 * Implements trace as file in debugfs.
 *
 * Content of this file is determined by the content
 * of the trace buffer, and user-defined function,
 * which translates messages in the trace buffer into strings.
 * 
 * Also provide functions for get/set some trace parameters.
 */


#include "trace_buffer.h"
#include <linux/module.h>

#include <linux/debugfs.h>

struct trace_file;

/*
 * Write message to the trace.
 * 
 * Message is an array of bytes 'msg' with size 'msg_size'.
 */

void trace_file_write_message(struct trace_file* tf,
    const void* msg, size_t msg_size);

/*
 * Reserve space in the buffer for writing message.
 * 
 * After call, pointer to the reserved space is saved in the 'msg'.
 * 
 * Return not-NULL identificator, which should be passed to
 * the trace_file_write_unlock() for commit writing.
 *
 * On error NULL is returned and 'msg' pointer shouldn't be used.
 * 
 * May be called in the atomic context.
 */
void* trace_file_write_lock(struct trace_file* tf,
    size_t size, void** msg);

/*
 * Commit message written after previous call
 * trace_buffer_write_lock().
 * 
 * May be called in the atomic context.
 */
void trace_file_write_unlock(struct trace_file* tf,
    void* id);


/*
 * Interpretator of the trace content.
 * 
 * This function should work simular to snptrinf and print string
 * into 'str' (no more than 'size' characters should be printed).
 * 
 * Application may interpret the message as some struct,
 * and perform formatting output of structure's fields.
 */

typedef int (*snprintf_message)(char* str, size_t size,
    const void* msg, size_t msg_size, int cpu, u64 ts, void* user_data);

/*
 * Create trace buffer with given buffer size and owerwrite mode.
 *
 * Create file in the given directory in debugfs, using which one can
 * read the trace.
 * Module 'm' is prevented from unload while trace file is opened.
 * 
 * 'print_message' is interpretator of trace buffer, 'user_data'
 * is the last parameter of this interpretator.
 */

struct trace_file* trace_file_create(
    size_t buffer_size, bool mode_overwrite,
    snprintf_message print_message, void* user_data);

/*
 * Remove trace buffer and trace file, which represent content of this
 * buffer.
 */
void trace_file_destroy(struct trace_file* tf);

ssize_t trace_file_read(struct trace_file *tf, char* __user buf,
    size_t size, bool can_block);

int trace_file_poll(struct trace_file *tf, struct poll_table* p,
    struct file* filp);

size_t trace_file_read_session(struct trace_file *tf, char* __user buf,
    size_t size, struct trace_session** s_p, bool can_block);

int trace_file_poll_session(struct trace_file *tf, struct poll_table* p,
    struct file* filp, struct trace_session* s);

/*
 * Reseting content of the trace file.
 */

void trace_file_reset(struct trace_file* tf);

/*
 * Return current size of trace buffer.
 * 
 * Note: This is not a size of the trace file.
 */

unsigned long trace_file_size(struct trace_file* tf);

/*
 * Reset content of the trace file and set new size for its buffer.
 * Return 0 on success, negative error code otherwise.
 */

int trace_file_size_set(struct trace_file* tf, unsigned long size);

/*
 * Return number of messages lost since the trace file 
 * creation/last reseting.
 */

unsigned long trace_file_lost_messages(struct trace_file* tf);

void trace_file_call_after_read(struct trace_file* tf,
    kedr_trace_callback_func func,
    struct kedr_trace_callback_head* callback_head);

#endif /* TRACE_FILE_H */
