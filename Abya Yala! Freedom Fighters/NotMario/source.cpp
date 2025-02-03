////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  Assignment:     PROG1266 Assignment 1
//  Instructor:     David Burchill
//  Year / Term:    Fall 2022
//  File name:      sfml.cpp
// 
//  Student name:   George P. Burdell
//  Student email:  gpburdell@hotmail.com
// 
//     I certify that this work is my work only, any work copied from Stack Overflow, textbooks, 
//     or elsewhere is properly cited. 
// 
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  BUG 
//  list any and all bugs in your code 
//  1. I didnt get abc to work because of xyz, I tried ijk...
//


//  sfml.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SFML/Graphics.hpp>
#include <iomanip>

#include "GameEngine.h"
 
int main()
{
	GameEngine game("../assets.txt");
	game.run();
	 
}