#include "tistdtypes.h"
#include "gen_sinus.h"
#include "sine_table.h"
#include "math.h"


#define PI 3.14159265

void gen_sinus_table(Int16 n, float a, float f, Int16 ph, float buffer[])
{
	int i = 0;
	int delta = f * SINE_TABLE_SIZE*4;
	int k = ph*delta;
	int mask = (SINE_TABLE_SIZE*4-1);

	if(ph!=0)
	{
		k+=delta;
	}


	for (i = 0; i < n; i++)
	{
		k = k & mask;
		if(k < SINE_TABLE_SIZE)
		{
			buffer[i] = a*p_sine_table[k];
		}
		else if(k < SINE_TABLE_SIZE*2)
		{
			buffer[i] = a*p_sine_table[SINE_TABLE_SIZE*2 - k - 1];
		}
		else if(k < 3*SINE_TABLE_SIZE)
		{
			buffer[i] = -a*p_sine_table[k - SINE_TABLE_SIZE*2];
		}
		else if(k < 4*SINE_TABLE_SIZE)
		{
			buffer[i] = - a*p_sine_table[SINE_TABLE_SIZE*4 - k - 1];
		}
		k+=delta;
	}

	/*int i = 0;
	int k=*ph;
	int delta = f * SINE_TABLE_SIZE * 4;
	int mask = (SINE_TABLE_SIZE-1);
	int mask1=(SINE_TABLE_SIZE*4-1);
	int n1=1024/delta;
	int n2=2048/delta;
	int n3=3072/delta;
	int n4=4096/delta;

	for (i = 0; i < n; i++)
	{
		if(k<1024)
		{
			*phh = *phh & mask;
			k = k & mask1;
			buffer[i] = a*p_sine_table[*phh];
			*phh+=delta;
			k+=delta;
			if(k>n1*delta)
			{
				*phh=0;
			}
		}

		else if(k>=1024 && k<2048)
		{
			*phh = *phh & mask;
			k = k & mask1;
			buffer[i] = a*p_sine_table[SINE_TABLE_SIZE-1-*phh];
			*phh+=delta;
			k+=delta;
			if(k>n2*delta)
			{
				*phh=0;
			}
		}
		else if(k>=2048 && k<3072)
		{
			*phh = *phh & mask;
			k = k & mask1;
			buffer[i] = -a*p_sine_table[*phh];
			*phh+=delta;
			k+=delta;
			if(k>n3*delta)
			{
				*phh=0;
			}
		}
		else if(k>=3072 && k<4096)
		{
			*phh = *phh & mask;
			k = k & mask1;
			buffer[i] = -a*p_sine_table[SINE_TABLE_SIZE-1-*phh];
			*phh+=delta;
			k+=delta;
			if(k>(n4-1)*delta)
			{
				*phh=0;
				k=0;
			}
		}
	}

	*ph=k;*/

}


void ADSR(float buffer[], Int16 n, Int16 current_offset, Int16 tone_duration)
{
	double trajanje_celog_tona = tone_duration*n; //u odbircima
	double attack_t=trajanje_celog_tona*0.1;
	double decay_t=trajanje_celog_tona*0.05;
	double release_t=trajanje_celog_tona*0.5;
	double sustain_t=trajanje_celog_tona*0.35;

	int i;
	double korak1=1/attack_t;
	double korak=(current_offset)/attack_t;

	double korak2=(trajanje_celog_tona-current_offset)/release_t;
	double korak22=1/release_t;


	for(i=0; i<n; i++)
	{
		if(current_offset<attack_t) //ton je poceo u attack fazi
		{
			buffer[i]=buffer[i]*korak;
			current_offset=current_offset+1;
			korak=korak+korak1;
		}
		else if(current_offset<attack_t+decay_t) //ton je poceo u decay fazi
		{
			buffer[i]=buffer[i]*(2-korak);
			current_offset=current_offset+1;
			korak=korak+(korak1/2);
		}
		else if(current_offset<attack_t+decay_t+sustain_t) //ton je poceo u sustain fazi
		{
			buffer[i]=buffer[i]*0.7;
			current_offset=current_offset+1;
		}
		else if(current_offset<attack_t+decay_t+sustain_t+release_t) //ton je poceo u release fazi
		{
			buffer[i]=buffer[i]*korak2*0.7;
			current_offset=current_offset+1;
			korak2=korak2-korak22;
		}
	}
}

Int16 quantB(float input, Uint16 B)
{
	Int16 Q = (1L << (B - 1));
	float output_float = floor(input * Q + 0.5);

	if(output_float == Q)
	{
		output_float = Q-1;
	}

	Int16 output_int = output_float;
	return output_int;
}

Int16 clipB(Int16 input, Uint16 B)
{
	Int16 max = (1L << (B-1)) - 1;
	Int16 min = - (1L << (B-1));
	Int16 output;
	if(input > max)
	{
		output = max;
	}
	else if (input < min)
	{
		output = min;
	}
	else
	{
		output = input;
	}

	return output;
}

float reconstructB(Int16 input, Uint16 B)
{
	float max_amp = (1L << 15);
	float input_float = input << (16-B);
	float output1 = input_float / max_amp;
	return output1;
}


float snr(float* signal, float* noise, Uint16 n)
{
	float ps=0.0;
	float pe=0.0;
	float snr=0.0;
	int i;
	for(i=0; i < n; i++)
	{
		ps += signal[i]*signal[i];
		pe += noise[i]*noise[i];
	}

	ps=ps/n;
	pe=pe/n;

	snr = 10*log10(ps/pe);

	return snr;
}



