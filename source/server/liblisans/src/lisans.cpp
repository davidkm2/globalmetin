#include "lisans.h"
// int lisans_ip[4] = {11000000, 10101000, 111000, 1101011}; // Local
int lisans_ip[4] = {10111001, 1110110, 10001111, 1001111}; // Makine

void getip(int add[])
{
	copy(lisans_ip, lisans_ip + 4, add);
}

bool Setup(string ip, int lip[])
{
	if (ip == cevir(lip))
		return true;
	else
		return false;
}

string cevir(int number[])
{
	string asd = "";
	for (int j = 0; j < 4; j++)
	{
		int d = 0, r, c = 0, p;
		while(number[j] > 0)
		{
			r = number[j] % 10;
			number[j] = number[j] / 10;
			p = pow(2, c);
			d = d + (r * p);
			c++;
		}
		char s[5];
		sprintf(s,"%d",d);
		asd += s;
		if (j < 3)
			asd += ".";
	}
	return asd;
}

