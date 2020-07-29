#ifndef DELAYUTILS_H_INCLUDED
#define DELAYUTILS_H_INCLUDED

#include <vector>
#include <cmath>

enum NextDelayType
{
    AddOne,
    NextPrime,
};

namespace DelayUtils
{
    /** Checks if number n is prime */
    inline static bool isPrime (int n)  
    {  
        // Corner cases  
        if (n <= 1)  return false;  
        if (n <= 3)  return true;  

        // This is checked so that we can skip   
        // middle five numbers in below loop  
        if (n%2 == 0 || n%3 == 0) return false;  

        for (int i=5; i*i<=n; i=i+6)  
            if (n%i == 0 || n%(i+2) == 0)  
                return false;  

        return true;  
    }

    /** Returns first prime number larger than n */
    inline static int nextPrime (int n)
    {
        // Base case 
        if (n <= 1) 
            return 2; 

        int prime = n; 
        bool found = false; 

        // Loop continuously until isPrime returns 
        // true for a number greater than n 
        while (!found) { 
            prime++; 

            if (isPrime(prime)) 
                found = true; 
        } 

        return prime; 
    }

    /**
        Generate orthogonal delay lengths.

        Note: DO NOT call this function from the audio thread,
        because it allocates memory.
    */
    static std::vector<int> generateDelayLengths (int numDelays, int start, float sep = 1.15f,
                                    NextDelayType nextType = NextPrime)
    {
        std::vector<int> lengths;
        lengths.push_back (start);
        float curLength = (float) start;

        for (int n = 1; n < numDelays; ++n)
        {
            curLength *= sep;
            
            if (nextType == AddOne)
                curLength += 1.0f;
            else if (nextType == NextPrime)
                curLength = (float) nextPrime ((int) curLength);

            lengths.push_back ((int) curLength);
        }

        return lengths;
    }

    /**
        Calculate the necessary feedback gain for a delay line to have a
        desired t60 [seconds]
    */
    static float calcGainForT60 (int delaySamples, float fs, float t60)
    {
        auto nTimes = t60 * fs / (float) delaySamples;
        return std::pow (0.001f, 1.0f / nTimes);
    }
}

#endif //DELAYUTILS_H_INCLUDED
