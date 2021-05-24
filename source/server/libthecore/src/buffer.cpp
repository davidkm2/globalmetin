#include "stdafx.h"
#include <algorithm>

// internal function forward
void buffer_realloc(LPBUFFER& buffer, int length);

LPBUFFER buffer_new(int size)
{
	if (size < 0)
		return NULL;

	buffer* buf = new buffer;

	buf->mem_size = size;
	buf->mem_data = new char[size];

	buffer_reset(buf);
	return buf;
}

void buffer_delete(LPBUFFER buf)
{
	if (buf == NULL)
		return;

	delete[] buf->mem_data;
	delete buf;
}

uint32_t buffer_size(LPBUFFER buf)
{
	return buf->length;
}

void buffer_reset(LPBUFFER buf)
{
	buf->next = NULL;
	buf->read_point = buf->mem_data;
	buf->write_point = buf->mem_data;
	buf->write_point_pos = 0;
	buf->length = 0;
	buf->flag = 0;
}

void buffer_write(LPBUFFER& buf, const void *src, int length)
{
	if (buf->write_point_pos + length >= buf->mem_size)
		buffer_realloc(buf, buf->mem_size + length + std::max<int>(1024, length));

	memcpy(buf->write_point, src, length);
	buffer_write_proceed(buf, length);
}

void buffer_read(LPBUFFER buf, void* data, int bytes)
{
	memcpy(data, buf->read_point, bytes);
	buffer_read_proceed(buf, bytes);
}

uint8_t buffer_byte(LPBUFFER buf)
{
	uint8_t val = *(uint8_t *)buf->read_point;
	buffer_read_proceed(buf, sizeof(uint8_t));
	return val;
}

uint16_t buffer_word(LPBUFFER buf)
{
	uint16_t val = *(uint16_t *)buf->read_point;
	buffer_read_proceed(buf, sizeof(uint16_t));
	return val;
}

uint32_t buffer_dword(LPBUFFER buf)
{
	uint32_t val = *(uint32_t *)buf->read_point;
	buffer_read_proceed(buf, sizeof(uint32_t));
	return val;
}

const void* buffer_read_peek(LPBUFFER buf)
{
	return (const void*)buf->read_point;
}

void buffer_read_proceed(LPBUFFER buf, int length)
{
	if (length == 0)
		return;

	if (length < 0)
	{
		sys_err("buffer_proceed: length argument lower than zero (length: %d)", length);

		return;
	}
	else if (length > buf->length)
	{
		sys_err("buffer_proceed: length argument bigger than buffer (length: %d, buffer: %d)", length, buf->length);

		length = buf->length;
	}

	// If the length to process is less than the buffer length, then the buffer should be left.
	if (length < buf->length)
	{
		// Leave write_point and pos unchanged, but increase read_point only.
		assert(buf->read_point + length - buf->mem_data <= buf->mem_size &&
			"Buffer overflow");

		buf->read_point += length;
		buf->length -= length;
	}
	else
	{
		buffer_reset(buf);
	}
}

void* buffer_write_peek(LPBUFFER buf)
{
	return buf->write_point;
}

void buffer_write_proceed(LPBUFFER buf, int length)
{
	buf->length += length;
	buf->write_point += length;
	buf->write_point_pos += length;
}

int buffer_has_space(LPBUFFER buf)
{
	return buf->mem_size - buf->write_point_pos;
}

void buffer_adjust_size(LPBUFFER& buf, int add_size)
{
	if (buf->mem_size >= buf->write_point_pos + add_size)
		return;

	sys_log(0, "buffer_adjust_size %d current %d/%d",
		add_size, buf->length, buf->mem_size);

	buffer_realloc(buf, buf->mem_size + add_size);
}

void buffer_realloc(LPBUFFER& buf, int length)
{
	int	i, read_point_pos;
	LPBUFFER temp;

	assert(length >= 0 && "buffer_realloc: length is lower than zero");

	if (buf->mem_size >= length)
		return;

	// i means the difference between the newly allocated size 
	// and the previous size, actually the size of the newly created memory.
	i = length - buf->mem_size;

	if (i <= 0)
		return;

	temp = buffer_new(length);

	sys_log(0, "reallocating buffer to %d, current %d",
		temp->mem_size, buf->mem_size);

	memcpy(temp->mem_data, buf->mem_data, buf->mem_size);

	read_point_pos = buf->read_point - buf->mem_data;

	// Reconnect write_point and read_point.
	temp->write_point = temp->mem_data + buf->write_point_pos;
	temp->write_point_pos = buf->write_point_pos;
	temp->read_point = temp->mem_data + read_point_pos;
	temp->flag = buf->flag;
	temp->next = NULL;
	temp->length = buf->length;

	buffer_delete(buf);
	buf = temp;
}