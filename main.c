//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta
// * APS / PNRSuOS
// *
// * Godina: 2022
// *
// * Zadatak:
// * Autor: Tamara Banovac EE30/2018
// *
// *
//////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "gen_sinus.h"
#include "sine_table.h"
#include "notes.h"
#include "fur_elise.h"
#include "math.h"


/* Frekvencija odabiranja */
#define SAMPLE_RATE 48000L
#define PI 3.14159265

static WAV_HEADER outputWAVhdr;
static WAV_HEADER inputWAVhdr;

/* Niz za smestanje odbiraka ulaznog signala */
#pragma DATA_ALIGN(InputBufferL,4)
Int16 InputBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(InputBufferR,4)
Int16 InputBufferR[AUDIO_IO_SIZE];

/* Niz za smestanje odbiraka izlaznog signala */
#pragma DATA_ALIGN(OutputBufferL,4)
Int16 OutputBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(OutputBufferR,4)
Int16 OutputBufferR[AUDIO_IO_SIZE];

float buffer1[AUDIO_IO_SIZE];
float buffer2[AUDIO_IO_SIZE];
float buffer3[AUDIO_IO_SIZE];
float buffer4[AUDIO_IO_SIZE];
float buffer5[AUDIO_IO_SIZE];

Int16 buffer1_int[AUDIO_IO_SIZE];
Int16 buffer2_int[AUDIO_IO_SIZE];
Int16 buffer3_int[AUDIO_IO_SIZE];
Int16 buffer4_int[AUDIO_IO_SIZE];
Int16 buffer5_int[AUDIO_IO_SIZE];

Int16 outputBuffer[AUDIO_IO_SIZE];

float reconstructed_sine[AUDIO_IO_SIZE];
float noise[AUDIO_IO_SIZE];
float SNRq[48]; //jer prvi ton ima 48 blokova pa da nadjem prosecan SNR

void main( void )
{   
	Int16 i, j,m,k,l,p;
	float amplitude = 1.0;
	float f0 = 0.0;
	Int16 phase1=0;
	Int16 phase2=0;
	Int16 phase3=0;
	Int16 phase4=0;
	Int16 phase5=0;
	float SNR=0.0;

	 /* Initialize BSL */
	 EZDSP5535_init( );

	 /* Initialise hardware interface and I2C for code */
	 aic3204_hardware_init();

	 aic3204_set_input_filename("../input1.wav");
	 aic3204_set_output_filename("../out_signal2.wav");

	 /* Initialise the AIC3204 codec */
	 aic3204_init();

	 aic3204_dma_init();

	 /* Citanje zaglavlja ulazne datoteke */
	 aic3204_read_wav_header(&inputWAVhdr);

	 /* Popunjavanje zaglavlja izlazne datoteke */
	 outputWAVhdr = inputWAVhdr;

	 /* Zapis zaglavlja izlazne datoteke */
	 aic3204_write_wav_header(&outputWAVhdr);


	for(i=192; i<furEliseLength; i++)
	{
		for(j=0; j<692; j++)
		{
			m=furEliseNotes0[j].time;
			k=furEliseNotes0[j].time+furEliseNotes0[j].duration;
			if(i>=m && i<k)
			{
				f0=note_to_freq(furEliseNotes0[j].note);
				phase1 = (i - furEliseNotes0[j].time) * AUDIO_IO_SIZE;
				gen_sinus_table(AUDIO_IO_SIZE, amplitude, f0/SAMPLE_RATE, phase1, buffer1);

				ADSR(buffer1, AUDIO_IO_SIZE, phase1, furEliseNotes0[j].duration);
				break;
			}

		}

		for(j=0; j<274; j++)
		{
			if(i>=furEliseNotes1[j].time && i<(furEliseNotes1[j].time+furEliseNotes1[j].duration))
			{
				f0=note_to_freq(furEliseNotes1[j].note);
				phase2 = (i - furEliseNotes1[j].time) * AUDIO_IO_SIZE;
				gen_sinus_table(AUDIO_IO_SIZE, amplitude, f0/SAMPLE_RATE, phase2, buffer2);

				ADSR(buffer2, AUDIO_IO_SIZE, phase2, furEliseNotes1[j].duration);
				break;
			}
		}

		for(j=0; j<60; j++)
		{
			if(i>=furEliseNotes2[j].time && i<(furEliseNotes2[j].time+furEliseNotes2[j].duration))
			{
				f0=note_to_freq(furEliseNotes2[j].note);
				phase3 = (i - furEliseNotes2[j].time) * AUDIO_IO_SIZE;
				gen_sinus_table(AUDIO_IO_SIZE, amplitude, f0/SAMPLE_RATE, phase3, buffer3);

				ADSR(buffer3, AUDIO_IO_SIZE, phase3, furEliseNotes2[j].duration);
				break;

			}
		}

		for(j=0; j<40; j++)
		{
			if(i>=furEliseNotes3[j].time && i<(furEliseNotes3[j].time+furEliseNotes3[j].duration))
			{
				f0=note_to_freq(furEliseNotes3[j].note);
				phase4 = (i - furEliseNotes3[j].time) * AUDIO_IO_SIZE;
				gen_sinus_table(AUDIO_IO_SIZE, amplitude, f0/SAMPLE_RATE, phase4, buffer4);

				ADSR(buffer4, AUDIO_IO_SIZE, phase4, furEliseNotes3[j].duration);
				break;
			}
		}

		for(j=0; j<6; j++)
		{
			if(i>=furEliseNotes4[j].time && i<(furEliseNotes4[j].time+furEliseNotes4[j].duration))
			{
				f0=note_to_freq(furEliseNotes4[j].note);
				phase5 = (i - furEliseNotes4[j].time) * AUDIO_IO_SIZE;
				gen_sinus_table(AUDIO_IO_SIZE, amplitude, f0/SAMPLE_RATE, phase5, buffer5);

				ADSR(buffer5, AUDIO_IO_SIZE, phase5, furEliseNotes4[j].duration);
				break;
			}
		}

		for (j = 0; j < AUDIO_IO_SIZE; j++)
		{
			buffer1_int[j]=quantB(buffer1[j],15);
			buffer1_int[j]=clipB(buffer1_int[j],14);

			buffer2_int[j]=quantB(buffer2[j],15);
			buffer2_int[j]=clipB(buffer2_int[j],14);

			buffer3_int[j]=quantB(buffer3[j],15);
			buffer3_int[j]=clipB(buffer3_int[j],14);

			buffer4_int[j]=quantB(buffer4[j],15);
			buffer4_int[j]=clipB(buffer4_int[j],14);

			buffer5_int[j]=quantB(buffer5[j],15);
			buffer5_int[j]=clipB(buffer5_int[j],14);

			outputBuffer[j]=buffer1_int[j]+buffer2_int[j]+buffer3_int[j]+buffer4_int[j]+buffer5_int[j];
		}

		aic3204_write_block(outputBuffer, outputBuffer);

		/*
		//SNR
		for(l = 0; l < AUDIO_IO_SIZE; l++)
		{
			reconstructed_sine[l] = reconstructB(buffer1_int[l], 15);
		}

		for(l = 0; l < AUDIO_IO_SIZE; l++)
		{
			noise[l] = reconstructed_sine[l]-buffer1[l];
		}

		SNRq[p]=snr(reconstructed_sine, noise, AUDIO_IO_SIZE); //to je za jedan blok
		p=p+1;

		if(p==48)
		{
			for(l=0;l<48;l++)
			{
				SNR+=SNRq[l];
			}

			SNR=SNR/48; //prosecan sum za prvi ton
		}
		//SNR
		*/
		for (j = 0; j < AUDIO_IO_SIZE; j++)
		{
			buffer1[j]=0;
			buffer2[j]=0;
			buffer3[j]=0;
			buffer4[j]=0;
			buffer5[j]=0;
		}

	}



	aic3204_disable();

    printf( "\n***Program has Terminated***\n" );
	SW_BREAKPOINT;
}
