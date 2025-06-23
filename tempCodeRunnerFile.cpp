#include<iostream>
using namespace std;

int hitungRekursif(int arr[], int n) {
    if (n == 0)
        return 0;

    int nilai = arr[n - 1];

    if (nilai % 2 == 1)
        return hitungRekursif(arr, n - 1) + nilai;
    else
        return hitungRekursif(arr, n - 1) - nilai;
}

int main(){
    int x[] = {11, 4, 7, 16, 9, 2, 13, 6, 5, 18, 3, 20, 1, 10, 8};
    int a = hitungRekursif(x, 15);
    cout << a;
}