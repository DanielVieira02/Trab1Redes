// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>  // Para a função strlen()
// #include <stdint.h>  // Para o tipo uint64_t

// int main(){
//     // pega um uint64_t e escreve todos os seus bits 
    
//     uint64_t teste = 117;
//     // for(int i = 63; i >= 0; i--){
//     //     printf("%d", teste & (1<<i) ? 1 : 0);
//     // }

//     // inserir o valor 117 em um pacote de 64 bits
//     void * pacote = (void *) calloc(1, 8);
//     // int a = 5;
//     // memcpy(pacote, &a, 8);
//     // printf("\n");
//     for(int i = 63; i >= 0; i--){
//         printf("%d", ((uint8_t*)pacote)[7 - i/8] & (1 << (i % 8)) ? 1 : 0);
//     }
//     // copia o valor de teste para o pacote
//     memcpy(pacote, &teste, 8);
    
//     // imprime os bits do pacote
//     printf("\n");
//     for(int i = 63; i >= 0; i--){
//         printf("%d", ((uint64_t*)pacote)[7 - i/8] & (1 << (i % 8)) ? 1 : 0);
//     }

//     // imprime o valor do pacote
//     printf("\n%lu\n", *(uint64_t *)pacote);
// }