
//#pragma warning(disable:4786)		// disable debug warning

#include <iostream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <iostream>					// for cout etc.
#include <vector>					// for vector class
#include <string.h>					// for string class
#include <algorithm>				// for sort algorithm
#include <time.h>					// for random seed
#include <math.h>					// for abs()
#include "defs.h"
#include "data.h"
#include "prototypes.h"


using namespace std;


#define popsize		6		// ga population size
#define maxiter		7		// maximum iterations
#define mutation_rate	0.75f		// mutation rate
#define mutation		RAND_MAX * mutation_rate
#define crossover_rate			0.20f
#define target_length    24
#define rand_num  ((float)rand()/(RAND_MAX+1))
string target;



struct ga_struct
{
	std::string str;
	int score;

	ga_struct():str(""), score(0){};
	ga_struct(std::string bts, int ints): str(bts), score(ints){}
};

//-----------Prototypes---------------------------

string init_population(int piece, int location);
int calc_fitness(int piece, std::string bits);
string Roulette(int total_fitness, ga_struct* population);
void Crossover(std::string &offspring1, std::string &offspring2);
void Mutate(std::string &bits);
string dec_to_bin(int dec);
int bin_to_dec(std::string bits);
int piece_value_final(std::string bits);
int location_value_final(std::string bits);
void fitness_sort(int *sorted_population);

//-----------------------------------------------


int GA(int piece ,int location)
{
	int i, totalfitness = 0;
	srand((int)time(NULL));

	ga_struct population[popsize];


	// Initialise random population with zero fitness score
	for(i=0; i <popsize; i++)
	{
		population[i].str = init_population(piece, location);
		population[i].score = 0;
	}


	//Run the loop until the max iterations are done

	for(i=0;i<maxiter;i++)
	{
		
		
	  // Calculate the finess
		for(i=0; i<popsize;i++)
		{
			population[i].score = calc_fitness(piece, population[i].str);

			totalfitness += population[i].score;
		}
//		cout<< "totalfitness:" << totalfitness << endl;

	  // create a new population by selecting two parents at a time and creating offspring
      // by applying crossover and mutation. Do this until the desired number of offspring
      // have been created. 
        
      //define some temporary storage for the new population we are about to create
        ga_struct temp[popsize];

        int cPop = 0;
     
        //loop until we have created POP_SIZE new chromosomes
        while (cPop < popsize)
        {
           // we are going to create the new population by grabbing members of the old population
           // two at a time via roulette wheel selection.
           string offspring1 = Roulette(totalfitness, population);
//		   cout<<"offspring1:"<<offspring1<<endl;
//		   cout<<"offspring1 value:"<<bin_to_dec(offspring1)<<endl;
           string offspring2 = Roulette(totalfitness, population);
//		   cout<<"offspring2:"<<offspring1<<endl;
//		   cout<<"offspring2 value:"<<bin_to_dec(offspring2)<<endl;

        //add crossover dependent on the crossover rate
        Crossover(offspring1, offspring2);
/*		   cout<<"offspring1 after crossover:"<<offspring1<<endl;
		   cout<<"offspring1 after crossovervalue:"<<bin_to_dec(offspring1)<<endl;
		   cout<<"offspring2 after crossover:"<<offspring2<<endl;
		   cout<<"offspring2 after crossovervalue:"<<bin_to_dec(offspring2)<<endl;
*/
           //now mutate dependent on the mutation rate
           Mutate(offspring1);
           Mutate(offspring2);
/*		   cout<<"offspring1 after mutation:"<<offspring1<<endl;
		   cout<<"offspring1 after mutation value:"<<bin_to_dec(offspring1)<<endl;
		   cout<<"offspring2 after mutation:"<<offspring2<<endl;
		   cout<<"offspring2 after mutation value:"<<bin_to_dec(offspring2)<<endl;
*/


           //add these offspring to the new population. (assigning zero as their
        //fitness scores)
           temp[cPop++] = ga_struct(offspring1, 0);
		   //cout<<offspring1<<endl;
           temp[cPop++] = ga_struct(offspring2, 0);
		   //cout<<offspring2<<endl;

        }//end loop   

        //copy temp population into main population array
/*		cout<<"final temp population:";
		for(int i=0; i< popsize;i++)
		{
			cout<<temp[i].str<<endl;
		}*/

        for (int i=0; i<popsize; i++)
      {
           population[i] = temp[i];
//		   cout<<i<<endl;
      } 
//		cout<<"final temp population xxxx:";
/*		for(int i=0; i< popsize;i++)
		{
			cout<<population[i].str<<endl;
		}*/


	  }


	//Sort the population
		int sorted_population[popsize];

		for(int i=0; i< popsize;i++)
		{
			sorted_population[i] = bin_to_dec(population[i].str);
		}
/*		cout<<"final to sort population values:";
		for(int i=0; i< popsize;i++)
		{
			cout<<sorted_population[i]<<endl;
		}*/

		fitness_sort(sorted_population);
/*		cout<<"final sorted population values:"<<endl;
		for(int i=0; i< popsize;i++)
		{
		cout<<sorted_population[i]<<endl;
		}*/

		for(int i=0; i< popsize;i++)
		{
			population[i].str = dec_to_bin(sorted_population[i]);
		}
/*		cout<<"final sorted population:"<<endl;
		for(int i=0; i< popsize;i++)
		{
		cout<<population[i].str<<endl;
		}*/

	
	//Split the population string into piece value and location value and return	
			
		int piece_value = piece_value_final(population[rand()%2 + 4].str);
		int location_value = location_value_final(population[rand()%2 + 4].str);

//		cout<<"piece_value:"<<piece_value<<endl;
//		cout<<"location_value:"<<location_value<<endl;
//		cout<<"return value:"<<piece_value + location_value<<endl;
		return piece_value + location_value;

}

string init_population(int piece, int location)
{
	int piece_value_GA, location_value;
	string piece_value_str, location_value_str, target;
	
	
// Initialise random population with zero fitness score
	switch (piece)
	{
		case PAWN: // Pawn
		piece_value_GA = rand()%40 + 80;
		location_value = rand()%10 + location;
//		cout<<"initial piece value:"<<piece_value_GA<<endl;
//		cout<<"initial location value:"<<location_value<<endl;
		break;

		case KNIGHT: // Knight
		piece_value_GA = rand()%60 + 270;
		location_value = rand()%10 + location;
//		cout<<"initial piece value:"<<piece_value_GA<<endl;
//		cout<<"initial location value:"<<location_value<<endl;
		break;

		case BISHOP: // Bishop
		piece_value_GA = rand()%60 + 270;
		location_value = rand()%10 + location;
//		cout<<"initial piece value:"<<piece_value_GA<<endl;
//		cout<<"initial location value:"<<location_value<<endl;
		break;

		case ROOK: // Rook
		piece_value_GA = rand()%60 + 470;
		location_value = rand()%10 - 5;
//		cout<<"initial piece value:"<<piece_value_GA<<endl;
//		cout<<"initial location value:"<<location_value<<endl;
		break;

		case QUEEN: // Queen
		piece_value_GA = rand()%100 + 850;
		location_value = rand()%10 + location;
//		cout<<"initial piece value:"<<piece_value_GA<<endl;
//		cout<<"initial location value:"<<location_value<<endl;
 		break;

		case KING: // King
		piece_value_GA = rand()%20 + 990;
		location_value = rand()%10 + location;
//		cout<<"initial piece value:"<<piece_value_GA<<endl;
//		cout<<"initial location value:"<<location_value<<endl;
		break;
	    }

		piece_value_str = dec_to_bin(piece_value_GA);
		location_value_str = dec_to_bin(location_value);
		
//------------------------------------------------------------------------------
//		cout<< " init_population_str:"<< piece_value_str + location_value_str << endl;
//		cout<< " piece_value_GA:"<< piece_value_GA<< endl;
//		cout<< " location_value"<< location_value<< endl;
//------------------------------------------------------------------------------

		return piece_value_str + location_value_str;
}

int calc_fitness(int piece, string bits)
{

	unsigned int fitness = 0;

	for (int i=0; i<popsize; i++) {

		switch(piece)
		{
			case PAWN:
				target = "000001100100000000010100";
			case KNIGHT:
				target = "000100101100000000000101";
			case BISHOP:
				target = "000100101100000000000101";
			case ROOK:
				target = "000111110100000000000000";
			case QUEEN:
				target = "001110000100000000000101";
			case KING:
				target = "001111101000000000000000";
		}

		for(int j = 0; j< target.size(); j++)
		{			
			fitness += abs(int(bits[j] - target[j]));
		}

	}
	
//-------------------------------------------------------
//	cout<< "calc_fitness:"<< target.size()<<endl;
//	cout<< "fitness:"<< fitness<<endl;
//-------------------------------------------------------

	return fitness;
	
  }

string Roulette(int total_fitness, ga_struct* population)
{
   //generate a random number between 0 & total fitness count
   float Slice = (float)(rand_num * total_fitness);
   
   //go through the chromosones adding up the fitness so far
   float FitnessSoFar = 0.0f;
   
   for (int i=0; i<popsize; i++)
   {
      FitnessSoFar += population[i].score;
      
      //if the fitness so far > random number return the chromo at this point
      if (FitnessSoFar >= Slice)
         return population[i].str;
   }

   return "";
}

void Crossover(string &offspring1, string &offspring2)
{

  //dependent on the crossover rate
  if (rand_num < crossover_rate)
  {
    //create a random crossover point
    int crossover = (int) (rand_num * target_length);

    string t1 = offspring1.substr(0, crossover) + offspring2.substr(crossover, target_length);
    string t2 = offspring2.substr(0, crossover) + offspring1.substr(crossover, target_length);

    offspring1 = t1; offspring2 = t2;              
  }
  /*
//---------------------------------
  cout<<"crossover executed"<<endl;
//----------------------------------
*/
}

void Mutate(string &bits)
{
   for (int i=0; i < bits.size(); i++)
   {
	  if(i >=2 && i < 12 && i>= 21 && i<24)
	  {
      if (rand_num < mutation_rate)
      {
         if (bits.at(i) == '1')

            bits.at(i) = '0';

         else

            bits.at(i) = '1';
      }
	  }
   } 

//   cout << "mutate string:" << bits.size()<< endl;
   return;
}

  
string dec_to_bin(int dec)
	{
		int rem, i = 0, bin;
		string x;
		ostringstream convert;   
	do
	{
		rem=dec%2;
		bin= rem;
		dec=dec/2;
		convert << bin;      
		x = convert.str();
		bin = 0;
	}while(dec>0);
//	cout<<x<<endl;
	string reverse = x;
	int len = x.length();
//	cout<<len<<endl;
	for (int n = 0;n < len; n++)
	{
	reverse[len - n - 1] = x[n];
	}
//	cout << reverse << endl;
	if(reverse.size() <= 12){
	while(reverse.size() < 12)
	{
		reverse = "0" + reverse;
	}
	}
	else if(reverse.size() > 12){
		while(reverse.size()<24)
			reverse = "0" + reverse;
	}
	return reverse;
	}


int bin_to_dec(string bits)
{
   int val          = 0;
   int value_to_add = 1;

   for (int i = bits.size(); i > 0; i--)
   {
      

      if (bits.at(i-1) == '1')

         val += value_to_add;

      value_to_add *= 2;
   
   }//next bit

   return val;
}

int piece_value_final(string bits)
{
	string value="000000000000";
	int piece_value;
	for(int i=0; i < bits.size()/2;i++){
	value[i] = bits[i];

	}
//	cout<<"piece_value_final:_str"<<value<<endl;	
	piece_value = bin_to_dec(value);
//	cout<<"piece_value_final:" << piece_value<< endl;
	return piece_value;
}

int location_value_final(string bits)
{
	string value="000000000000";
	int location_value,j=0;	
	for(int i= 12; i < bits.size();i++){
	value[j] = bits[i];
	j++;
	}
//	cout<<"location_value_final_str:" << value<< endl;
	location_value = bin_to_dec(value);
//	cout<<"location_value_final:" << location_value<< endl;
	return location_value;
}



void fitness_sort(int *sorted_population)
{
	int temp;

	for(int x = 0; x < popsize; x ++) {
            for(int y = x+1; y < popsize; y ++) {
                if(sorted_population[y] < sorted_population[x]) {
                    temp = sorted_population[x];
                    sorted_population[x] = sorted_population[y];
                    sorted_population[y] = temp;
                   
                }
            }
        }

}




























































