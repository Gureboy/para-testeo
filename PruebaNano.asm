.section .data
buffer:      .space 1024            // Espacio para 1024 bytes de buffer
buffer_size: .word 1024             // Tamaño del buffer
message:     .asciz "Procesando paquete...\n" //Mensaje para mostrar de paquete
.section .bss
processed:   .space 4               // Contador de paquetes procesados

.section .text
.global _start

_start:
    // Inicializar el buffer
    mov x0, #0                      // Inicializa el índice del buffer
    mov x1, #0                      // Inicializa el contador de paquetes

receive_packet:
    // Simula recibir datos
    adr x2, buffer                  // Dirección del buffer
    add x2, x2, x0                  // Posición actual del buffer
    mov x3, #32                     // Longitud del paquete 
    bl receive_data                 // Llama a la función para recibir datos

    // Procesar el paquete
    bl process_packet               // Llama a la función de procesamiento
    add x0, x0, x3                  // Avanza en el buffer
    add x1, x1, #1                  // Incrementa el contador de paquetes

    // Chequear si el buffer está lleno
    adr x4, buffer_size             // Obtén el tamaño total del buffer
    ldr w5, [x4]                    // Carga el tamaño en un registro
    cmp x0, x5                      // Compara el índice actual con el tamaño
    blt receive_packet              // Si no está lleno, recibe el próximo paquete

    // Reinicia el buffer y repite
    mov x0, #0                      // Reinicia el índice
    b receive_packet                // Regresa al inicio del loop

// Función para simular la recepción de datos
receive_data:
    // Aca se implementan los datos aún no se que poner
    ret

// Función para procesar datos
process_packet:
    // Imprime un mensaje de procesamiento
    adr x6, message                 // Dirección del mensaje
    bl print_message                // Llama a la función para imprimir
    ret

// Función para imprimir un mensaje 
print_message:
    mov x0, #1                      // STDOUT
    mov x1, x6                      // Dirección del mensaje
    mov x2, #23                     // Longitud del mensaje
    mov x8, #64                     // Syscall para escribir
    svc 0                           // Llama al sistema
    ret
