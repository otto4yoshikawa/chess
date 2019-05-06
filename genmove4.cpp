#include <stdio.h>
#include"genmove4.h"
#include <string.h>

 GENMOVE::GENMOVE(){

 step=1;}

 bool GENMOVE::InputMove(char *mrs){
 step++:
   return true;
 }
 bool GENMOVE::ComputerPlay(char *mes){
	   step++;
	   if(step>10){

	   strcpy(mes,"game end");
	   return true;
	   }

	  return false
 }

