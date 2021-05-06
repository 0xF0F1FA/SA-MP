
#include "main.h"

#include <ttmath/ttmath.h>

#define ROTL(x,y) (((x) << (y)) | ((x) >> (32-(y))))

typedef struct {
	unsigned int h0, h1, h2, h3, h4;
	unsigned int lcount;
	unsigned int hcount;
	unsigned char block[64];
	unsigned int block_index;
	bool computed;
	bool corrupted;
} SHA1Context_;

class CSHA1_ : public SHA1Context_
{
public:
	CSHA1_()
	{
		Reset();
	}

	void Reset()
	{
		lcount = 0;
		hcount = 0;
		block_index = 0;
		h0 = 0x67452301;
		h1 = 0xEFCDAB89;
		h2 = 0x98BADCFE;
		h3 = 0x10325476;
		h4 = 0xC3D2E1F0;
		computed = false;
		corrupted = false;
	}

	void Transform()
	{
		unsigned int a, b, c, d, e, i, t;
		unsigned int w[80] = { 0 };

		for (i = 0; i < 16; i++)
		{
			w[i] = block[i * 4] << 24;
			w[i] |= block[i * 4 + 1] << 16;
			w[i] |= block[i * 4 + 2] << 8;
			w[i] |= block[i * 4 + 3];
		}

		for (i = 16; i < 80; i++)
		{
			w[i] = ROTL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		}

		a = h0;
		b = h1;
		c = h2;
		d = h3;
		e = h4;

		for (i = 0; i < 20; i++)
		{
			t = ROTL(a, 5) + ((b & c) | ((~b) & d)) + e + w[i] + 0x5A827999;
			e = d;
			d = c;
			c = ROTL(b, 30);
			b = a;
			a = t;
		}

		for (i = 20; i < 40; i++)
		{
			t = ROTL(a, 5) + (b ^ c ^ d) + e + w[i] + 0x6ED9EBA1;
			e = d;
			d = c;
			c = ROTL(b, 30);
			b = a;
			a = t;
		}

		for (i = 40; i < 60; i++)
		{
			t = ROTL(a, 5) + ((b & c) | (b & d) | (c & d)) + e + w[i] + 0x8F1BBCDC;
			e = d;
			d = c;
			c = ROTL(b, 30);
			b = a;
			a = t;
		}

		for (i = 60; i < 80; i++)
		{
			t = ROTL(a, 5) + (b ^ c ^ d) + e + w[i] + 0xCA62C1D6;
			e = d;
			d = c;
			c = ROTL(b, 30);
			b = a;
			a = t;
		}

		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
		block_index = 0;
	}

	void Final()
	{
		if (block_index <= 55)
		{
			block[block_index++] = 0x80;
			while (block_index < 56)
			{
				block[block_index++] = 0;
			}
		}
		else
		{
			block[block_index++] = 0x80;
			while (block_index < 64)
			{
				block[block_index++] = 0;
			}

			Transform();

			while (block_index < 56)
			{
				block[block_index++] = 0;
			}
		}

		block[56] = hcount >> 24;
		block[57] = hcount >> 16;
		block[58] = hcount >> 8;
		block[59] = hcount;
		block[60] = lcount >> 24;
		block[61] = lcount >> 16;
		block[62] = lcount >> 8;
		block[63] = lcount;

		Transform();
	}

	bool GetHash(unsigned int* states)
	{
		if (corrupted) return 0;

		if (!computed)
		{
			Final();
			computed = 1;
		}

		states[0] = h0;
		states[1] = h1;
		states[2] = h2;
		states[3] = h3;
		states[4] = h4;

		return 1;
	}

	void Update(unsigned char* data, unsigned int length)
	{
		if (!length)
			return;

		if (computed || corrupted)
		{
			corrupted = true;
		}
		else
		{
			while (length-- && !corrupted)
			{
				block[block_index] = *data;

				block_index += 1;
				lcount += 8;

				if (lcount == 0)
				{
					hcount += 1;
					if (hcount == 0)
					{
						corrupted = true;
					}
				}

				if (block_index == 64)
				{
					Transform();
				}

				data++;
			}
		}
	}

	void Update(char* data, int len)
	{
		Update((unsigned char*)data, len);
	}

	/*void Update(char c)
	{
		Update((unsigned char*)&c, 1);
	}

	void Update(unsigned char c)
	{
		Update((unsigned char*)&c, 1);
	}

	void Update(char* data)
	{
		while (*data)
		{
			Update(*data);
			data++;
		}
	}*/
};

char bits_to_char(unsigned char b)
{
	char c = b + 0x30;
	return (c > 0x39) ? (b + 0x37) : c;
}

void bytes_to_string(char* output, unsigned char* state_input)
{
	int x = 0;
	while (x < 40)
	{
		output[x++] = bits_to_char(*state_input >> 4);
		output[x++] = bits_to_char(*state_input & 0xF);
		state_input++;
	}
	output[x] = '\0';
}

void GenerateComputerID(char* dest, char* src, int unk)
{
	unsigned int states[5];
	//unsigned int result_states[5];
	unsigned char* state_bytes;
	CSHA1_ sha1;

	states[0] = 0;
	states[1] = 0;
	states[2] = 0;
	states[3] = 0;
	states[4] = 0;

	sha1.Update(src, strlen(src));
	sha1.GetHash(states);

	//result_states[0] = states[0];
	//result_states[1] = states[1];
	//result_states[2] = states[2];
	//result_states[3] = states[3];
	//result_states[4] = states[4];

	state_bytes = (unsigned char*)states;

	BYTE b0, b1, b2, b3;
	for (int i = 0; i < 20; i++)
	{
		b0 = state_bytes[i] & 3;
		b1 = (state_bytes[i] >> 2) & 3;
		if (b0 <= b1)
			b2 = b0 | 4 * b1;
		else
			b2 = b1 | 4 * b0;

		b0 = (state_bytes[i] >> 4) & 3;
		b1 = state_bytes[i] >> 6;
		if (b0 <= b1)
			b3 = b0 | 4 * b1;
		else
			b3 = b1 | 4 * b0;

		state_bytes[i] = b2 | 16 * b3;
	}

	if (!pNetGame)
		unk--;

	char output[44];
	bytes_to_string(output, state_bytes);

	ttmath::UInt<100> m;
	m.FromString(output, 16);
	m.MulInt(unk);
	std::string temp = m.ToString(16);

	strcpy(dest, temp.c_str());
}
