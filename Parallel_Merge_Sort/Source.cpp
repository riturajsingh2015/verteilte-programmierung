#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "omp.h"
#include <string>
using namespace std;
#define Sequentielle_Schwelle 1024
#define MAX(a,b) (((a)>(b))?(a):(b))


///Sequentialles Mergesort funcktions /////////
bool Wenig(int x, int y)
{
    if (x < y) return true;
    else return false;
}
/*
Obwohl Sub-Arrays in einer sequentiellen Version zusammengeführt werden
benachbart sind, was bei paralleler Version nicht der Fall ist
und daher müssen Subarray-Grenzen angegeben werden ausdrücklich.
*/
void S_Merge(int* to, int* temp, int lowX, int highX, int lowY, int highY, int lowTo)
{
    int highTo = lowTo + highX - lowX + highY - lowY + 1;
    for (; lowTo <= highTo; lowTo++) // Gehe alle Elemente in den linken und rechten Arrays durch
    {
        if (lowX > highX) // Wenn Sie alle Elemente des linken Teils gescannt haben, fügen Sie Elemente vom rechten Teil in das ursprüngliche Array ein.
            to[lowTo] = temp[lowY++];
        else if (lowY > highY) // Wenn Sie alle Elemente des rechten Teils gescannt haben, fügen Sie Elemente vom linken Teil in das ursprüngliche Array ein.
            to[lowTo] = temp[lowX++];
        else // Suchen Sie ein kleineres Element im linken und rechten Array und fügen Sie es in das ursprüngliche Array ein
            to[lowTo] = Wenig(temp[lowX], temp[lowY]) ? temp[lowX++] : temp[lowY++];
    }
}

void S_MergeSort(int* to, int* temp, int low, int high)
{
    if (low >= high)
        return;
    int mid = (low + high) / 2;
    // Auf dem Weg den Rekursionsbaum hinunter haben beide Arrays
    //die gleichen Daten, damit wir sie wechseln können.
    
    
    S_MergeSort(temp, to, low, mid);
    S_MergeSort(temp, to, mid + 1, high);
    // Sobald das temporäre Array zwei sortierte Subarrays enthält, werden sie zu einem Zielarray zusammengeführt.

    S_Merge(to, temp, low, mid, mid + 1, high, low);
    //Auf dem Weg nach oben sind wir entweder als Zielarray fertig
    //    ist das ursprüngliche Array und enthält jetzt erforderlich
    //    Sub - Array sortiert oder es ist das temporäre Array vom vorherigen
    //    Schritt und enthält kleinere Sub - Array, die sein wird
    //    aus dem vorherigen Schritt in das Zielarray eingefügt
    //    (Das ist das temporäre Array dieses Schritts und so wir
    //        kann seinen Inhalt zerstören).
}

// Sucht nach Index das erste Element im niedrigen bis hohen Bereich, das streng größer als der angegebene Wert ist
//und alle Elemente innerhalb des angegebenen Bereichs sind kleiner
//oder gleich dem Index des Elements neben dem Bereich ist
//ist zurückgekommen.

int Binare_Suche(int* from, int low, int high, int key)
{
    high = MAX(low, high + 1);
    while (low < high)
    {
        int mid = (low + high) / 2;
        if (Wenig(from[mid], key))
            low = mid + 1;
        else
            high = mid;
    }
    return low;
}


///Parallel mergesort functions//

// Da die parallele Zusammenführung selbst rekursiv ist, ist derselbe Mechanismus
//Für Aufgaben wird eine Nummernbegrenzung verwendet.
void P_Merge(int* to, int* temp, int lowX, int highX, int lowY, int highY, int lowTo)
{
    int lengthX = highX - lowX + 1;
    int lengthY = highY - lowY + 1;


    if (lengthX + lengthY <= Sequentielle_Schwelle)
    {
        //Bei kleinen auf den sequentiellen Algorithmus zurückgreifen
        //Unterproblem oder tiefe Rekursion.
        S_Merge(to, temp, lowX, highX, lowY, highY, lowTo);
        return;
    }
  
    if (lengthX < lengthY)
    {
        // Stellen Sie sicher, dass der X-Bereich nicht weniger als der Y-Bereich und ist
        // tauschen Sie sie bei Bedarf aus.
        P_Merge(to, temp, lowY, highY, lowX, highX, lowTo);
        return;
    }

    // Ermittelt den Median des X-Subarrays. Als X-Subarray gilt
    //sortiert bedeutet dies, dass X[low ..modX - 1] Wenig als oder gleich dem Median und X[modx + 1 ..high] größer oder gleich dem Median sind.
    
    int midX = (lowX + highX) / 2;
    // Suchen Sie das Element im Y-Unterarray, das streng ist
    // größer als X[midX].Wieder wie Y Sub - Array ist
    // sortiert Y[lowY ..midY - 1] sind Wenig als oder gleich
    // bis X[midX] und Y[midY ..highY] sind größer als  X[midX].


    /// Binary-Search ist in Rechten Array gemacht nach der Mittel-Element des Linken Array
    int midY = Binare_Suche(temp, lowY, highY, temp[midX]);  

    // Jetzt können wir die endgültige Position im Ziel berechnen
    // Array des Medians des X - Subarrays.
    int midTo = lowTo + midX - lowX + midY - lowY;
    to[midTo] = temp[midX];
    // Der Rest besteht darin, X [lowX .. midX - 1] mit zusammenzuführen
    // Y[lowY ..midY - 1] und X[midx + 1 ..highX]
    // mit Y[midY ..highY] vor und nach
    // Median jeweils im Zielarray.Wie
    // Paare sind von ihrer endgültigen Position abhängig
    // Perspektive können sie parallel zusammengeführt werden.
    
#pragma omp parallel sections
    {
#pragma omp section
        P_Merge(to, temp, lowX, midX - 1, lowY, midY - 1, lowTo);
#pragma omp section
        P_Merge(to, temp, midX + 1, highX, midY, highY, midTo + 1);
    }   

}



void P_MergeSort(int* to, int* temp, int low, int high)
{
   
  
    if (high - low + 1 <= Sequentielle_Schwelle)
    {    

        S_MergeSort(to, temp, low, high);
        return;
    }
    
    int mid = (low + high) / 2;
    // Dieselbe Umschalttechnik für Ziel- / Temp-Arrays
    // wie in sequentieller Version gilt parallel
    // Ausführung. Sub-Arrays sind unabhängig und können somit
    // parallel sortiert werden.

    #pragma omp parallel sections
        {
        #pragma omp section
                P_MergeSort(temp, to, low, mid);// Links thread
        #pragma omp section
                P_MergeSort(temp, to, mid + 1, high);// Rechts thread
        }


    // Sobald beide Takes abgeschlossen waren, wurde die Zusammenführung sortiert
    // Sub-Arrays parallel.
    P_Merge(to, temp, low, mid, mid + 1, high, low);
    
}



// Hybrid
void H_MergeSort(int* to, int* temp, int low, int high)
{
    if (high - low + 1 <= Sequentielle_Schwelle)
    {
      
        S_MergeSort(to, temp, low, high);
        return;
    }

    int mid = (low + high) / 2;
    // Dieselbe Umschalttechnik für Ziel- / Temp-Arrays
    // wie in sequentieller Version gilt parallel
    // Ausführung. Sub-Arrays sind unabhängig und können somit
    // parallel sortiert werden.

#pragma omp parallel sections
    {
#pragma omp section
        H_MergeSort(temp, to, low, mid);// Links thread
#pragma omp section
        H_MergeSort(temp, to, mid + 1, high);// Rechts thread
    }


    // Sobald beide Takes abgeschlossen waren, wurde die Zusammenführung sortiert
    // Sub-Arrays parallel.   
    S_Merge(to, temp, low, mid, mid + 1, high, low);
}


void MergeSort(int* array, int low, int high, int choice)
{
	// Diese Sie eine Kopie des Interessierten Arrays. Umschalten zwischen
    // Das abwehrende Array und seine Kopie lassen sich schützen
    // wider Array-Zuordnungen und Kopieren von Daten.

	int array_size = high - low + 1;
    double startt, endt;
	int* copy = new int[array_size];
	memcpy(copy, array, array_size * sizeof(int)); 
    startt = omp_get_wtime();
	if (choice == 1)
        S_MergeSort(array, copy, low, high);
    else if(choice==2)
        H_MergeSort(array, copy, low, high);
    else if (choice == 3)
        P_MergeSort(array, copy, low, high);   
    endt = omp_get_wtime();
    std::cout << "Zeit zum Sortieren in Sekunden : " << endt - startt << "\n\n";
}


//Dienstprogrammfunktionen //

void erstell_A(int* A, int A_size)
{
    int R_MAX = 1000; // Ändern Sie diesen Wert, um den Zahlenbereich zu vergrößern 
    srand((unsigned)time(NULL));   // Die Initialisierung sollte nur einmal aufgerufen werden.
    for (int i = 0; i < A_size; i++)
    {
        //Initialisieren Sie das Array mit zufälligen Werten
        A[i] = rand() % R_MAX;      // Gibt eine pseudozufällige Ganzzahl zwischen 0 und RAND_MAX zurück.
    }

}

void druck_A(int* A, int A_size)
{
    // Diese funktion drukt Array A nur wenn GroBe von A ist <=50
    if (A_size <= 50) {
        std::cout << "[ ";
        for (int i = 0; i < A_size; i++)
        {
            std::cout << A[i] << " ";
        }
        std::cout << "]\n\n";
    }
}





int main() {    
 

        int A_size , version;
        
        std::cout << "Geben Sie die GroBe von Array A ein ";
        std::cin >> A_size;
        int* A = new int[A_size];
        erstell_A(A, A_size);
        std::cout << "\n Array A erstellt\n";
        std::cout << "\nGeben Sie die Mergesortsversion ein ";
        std::cout << "\n 1--> S_MergeSort , 2--> H_MergeSort , 3--> P_MergeSort ";
        std::cin >> version;              
       
        druck_A(A, A_size);
       // Wahl letzte parameter des Mergesorts
        // 1--> S_MergeSort , 2--> H_MergeSort , 3--> P_MergeSort
        MergeSort(A, 0, A_size - 1, version);
        druck_A(A, A_size);

   
	return 0;
}