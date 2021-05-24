#pragma once
#include <cstdint>

#define SAFE_BUFFER_DELETE(buf)		{ if(buf) { buffer_delete(buf); buf = NULL; } }

typedef struct buffer	BUFFER;
typedef struct buffer *	LPBUFFER;

struct buffer
{
	buffer* next;

	char* write_point;
	int write_point_pos;

	const char* read_point;
	int length;

	char* mem_data;
	int mem_size;

	uint32_t flag;
};

extern LPBUFFER buffer_new(int size); // Create new buffer
extern void buffer_delete(LPBUFFER buffer); // Deleting a buffer
extern void buffer_reset(LPBUFFER buffer); // Initialize buffer lengths

extern uint32_t buffer_size(LPBUFFER buffer); // Length remaining in buffer
extern int buffer_has_space(LPBUFFER buffer); // Return a usable length

extern void buffer_write(LPBUFFER& buffer, const void* src, int length); //Write to the buffer.
extern void buffer_read(LPBUFFER buffer, void* data, int bytes); // Read from the buffer.
extern uint8_t buffer_get_byte(LPBUFFER buffer);
extern uint16_t buffer_get_word(LPBUFFER buffer);
extern uint32_t	buffer_get_dword(LPBUFFER buffer);

// The buffer_proceed function needs to return a read pointer to buffer_peek
// Write when you need to be notified when processing is done.
// (In case of buffer_read, buffer_get_ * series, it is processed but peek
// when it can not be so)
extern const void* buffer_read_peek(LPBUFFER buffer); // Return location to read
extern void buffer_read_proceed(LPBUFFER buffer, int length); // End of length processing

// Similarly, write_peek gets the write location,
// Use buffer_write_proceed.
extern void* buffer_write_peek(LPBUFFER buffer); // Return the writing position
extern void buffer_write_proceed(LPBUFFER buffer, int length); // Increase length only.

extern void buffer_adjust_size(LPBUFFER & buffer, int add_size); // Get the size to add by add_size
