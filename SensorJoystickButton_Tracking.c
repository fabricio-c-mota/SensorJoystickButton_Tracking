#include "pico/stdlib.h" // Inclui a biblioteca padrão do Raspberry Pi Pico, que fornece funções básicas de entrada/saída.
#include "hardware/adc.h" // Inclui a biblioteca para controle do conversor analógico-digital (ADC) do Pico.
#include "pico/cyw43_arch.h" // Inclui a biblioteca para suporte à arquitetura CYW43, que é usada para conectividade Wi-Fi.
#include <stdio.h> // Inclui a biblioteca padrão de entrada/saída para funções como printf.
#include <string.h> // Inclui a biblioteca para manipulação de strings.
#include <stdlib.h> // Inclui a biblioteca para funções de alocação de memória e conversão de tipos.
#include "lwip/pbuf.h" // Inclui a biblioteca LWIP para manipulação de buffers de pacotes.
#include "lwip/tcp.h" // Inclui a biblioteca LWIP para suporte a protocolos TCP.
#include "lwip/netif.h" // Inclui a biblioteca LWIP para manipulação de interfaces de rede, permitindo acesso a netif_default e IP.

// Configurações de Wi-Fi
#define WIFI_SSID "RedmiNote13Pro" // Define o SSID da rede Wi-Fi à qual o dispositivo irá se conectar.
#define WIFI_PASSWORD "shepherd" // Define a senha da rede Wi-Fi.

// Definição dos pinos dos LEDs
#define LED_PIN CYW43_WL_GPIO_LED_PIN
#define BUTTON_A 5 // GPIO5 - Botão A

#define JOYSTICK_X_PIN 0 // Define o pino para a leitura do eixo X do joystick.
#define JOYSTICK_Y_PIN 1 // Define o pino para a leitura do eixo Y do joystick.

#define ADC_MAX ((1 << 12) - 1) // Define o valor máximo que o ADC pode ler, que é 4095 para um ADC de 12 bits.

// Variável global para armazenar a última direção detectada
static char last_direction[16] = "Norte"; // Declara uma string estática para armazenar a última direção detectada, inicializada como "Norte".

// Função para detectar a direção com base nos valores X e Y
const char* get_direction(uint adc_x_raw, uint adc_y_raw) { 
    // Verifica se os valores de entrada estão nas regiões correspondentes e retorna a direção apropriada
    if (adc_y_raw < ADC_MAX / 4 && adc_x_raw < ADC_MAX / 4) {
        return "Sudoeste"; // Se ambos os valores estão na parte inferior esquerda, retorna "Sudoeste".
    } else if (adc_y_raw < ADC_MAX / 4 && adc_x_raw > 3 * ADC_MAX / 4) {
        return "Noroeste"; // Se Y está na parte inferior e X na parte superior direita, retorna "Noroeste".
    } else if (adc_y_raw > 3 * ADC_MAX / 4 && adc_x_raw < ADC_MAX / 4) {
        return "Sudeste";  // Se Y está na parte superior e X na parte inferior esquerda, retorna "Sudeste".
    } else if (adc_y_raw > 3 * ADC_MAX / 4 && adc_x_raw > 3 * ADC_MAX / 4) {
        return "Nordeste"; // Se ambos os valores estão na parte superior direita, retorna "Nordeste".
    } else if (adc_y_raw < ADC_MAX / 3) {
        return "Oeste"; // Se Y está na parte inferior, retorna "Oeste".
    } else if (adc_y_raw > 2 * ADC_MAX / 3) {
        return "Leste"; // Se Y está na parte superior, retorna "Leste".
    } else if (adc_x_raw < ADC_MAX / 3) {
        return "Sul"; // Se X está na parte inferior, retorna "Sul".
    } else if (adc_x_raw > 2 * ADC_MAX / 3) {
        return "Norte"; // Se X está na parte superior, retorna "Norte".
    } else {
        return "Centro"; // Se não se encaixa em nenhuma das condições, retorna "Centro".
    }
}

static char button_status[16] = "Solto"; // Declara uma string estática para armazenar o status do botão, inicializada como "Solto".

// Função para verificar o status do botão
void check_button_status() {
    // Verifica se o botão está pressionado (considerando que o estado baixo indica pressionado)
    if (!gpio_get(BUTTON_A)) { 
        strncpy(button_status, "Pressionado", sizeof(button_status) - 1); // Se pressionado, atualiza o status para "Pressionado".
    } else {
        strncpy(button_status, "Solto", sizeof(button_status) - 1); // Se não pressionado, atualiza o status para "Solto".
    }
    button_status[sizeof(button_status) - 1] = '\0'; // Garante a terminação da string para evitar overflow.
}

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)  // Verifica se o buffer de recebimento está vazio (p é NULL)
    {
        tcp_close(tpcb); // Fecha a conexão TCP se o buffer estiver vazio.
        tcp_recv(tpcb, NULL); // Remove o callback de recebimento.
        return ERR_OK; // Remove o callback de recebimento.
    }

    // Aloca memória para armazenar a requisição recebida e copia os dados do buffer
    char *request = (char *)malloc(p->len + 1); // Aloca memória para a requisição, incluindo espaço para o terminador nulo.
    memcpy(request, p->payload, p->len); // Copia os dados do payload do buffer para a variável request.
    request[p->len] = '\0'; // Adiciona o terminador nulo ao final da string.

    // Leitura da temperatura interna
    adc_select_input(4); // Seleciona o canal do ADC (Analog-to-Digital Converter) que está conectado ao sensor de temperatura.
    uint16_t raw_value = adc_read(); // Lê o valor bruto do ADC, que representa a tensão medida pelo sensor de temperatura.
    const float conversion_factor = 3.3f / (1 << 12); // Define o fator de conversão para transformar o valor bruto em volts. 
    // O valor 3.3f representa a tensão de referência do ADC (3.3V) e (1 << 12) é 4096, que é o número máximo de valores que um ADC de 12 bits pode representar.
    float temperature = 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f; // Calcula a temperatura em graus Celsius.
    // A fórmula ajusta a leitura do ADC para obter a temperatura real, onde 27.0f é uma constante que representa a temperatura ambiente em graus Celsius,
    // 0.706f é um valor de offset e 0.001721f é a sensibilidade do sensor de temperatura.

    // Seleciona o canal do ADC para ler o eixo X do joystick
    adc_select_input(JOYSTICK_X_PIN); // Seleciona o pino do joystick correspondente ao eixo X.
    uint adc_x_raw = adc_read(); // Lê o valor bruto do eixo X do joystick usando o ADC.
    // Seleciona o canal do ADC para ler o eixo Y do joystick
    adc_select_input(JOYSTICK_Y_PIN); // Seleciona o pino do joystick correspondente ao eixo Y.
    uint adc_y_raw = adc_read(); // Lê o valor bruto do eixo Y do joystick usando o ADC.
    // Determina a direção com base nos valores lidos dos eixos X e Y do joystick
    const char* direction = get_direction(adc_x_raw, adc_y_raw); // Chama a função get_direction para obter a direção correspondente aos valores dos eixos.
    // Atualiza a última direção detectada com a nova direção lida
    strncpy(last_direction, direction, sizeof(last_direction) - 1); // Copia a nova direção para a variável last_direction, garantindo que não ocorra overflow
    last_direction[sizeof(last_direction) - 1] = '\0'; // Garante que a string last_direction esteja sempre terminada em nulo.

    check_button_status(); // Chama a função check_button_status para verificar se o botão está pressionado ou solto.

    // Cria a resposta HTML
    char html[1024]; // Declara um buffer para armazenar a resposta HTML. Pode ser alterado conforme a necessidade da aplicação.

    snprintf(html, sizeof(html), // Formata a string HTML e armazena no buffer html.
             "HTTP/1.1 200 OK\r\n" // Formata a string HTML e armazena no buffer html.
             "Content-Type: text/html\r\n" // Define o tipo de conteúdo como HTML.
             "\r\n" // Indica o fim dos cabeçalhos HTTP.
             "<!DOCTYPE html>\n" // Declara o tipo de documento HTML.
             "<html>\n" // Início do documento HTML.
             "<head>\n" // Início da seção de cabeçalho do HTML.
             "<title>Tracking...</title>\n" // Define o título da página.
             "<style>\n" // Início da seção de estilo CSS.
             "body { font-family: 'Roboto', sans-serif; background-color: #f4f4f4; color: #333; text-align: center; margin: 0; padding: 20px; }\n"
             "h1 { font-size: 48px; margin-bottom: 20px; color: #4A90E2; }\n"
             "button { font-size: 24px; margin: 10px; padding: 15px 30px; border-radius: 5px; border: none; background-color: #4A90E2; color: white; cursor: pointer; transition: background-color 0.3s; }\n"
             "button:hover { background-color: #357ABD; }\n"
             ".temperature { font-size: 36px; margin-top: 20px; color: #333; }\n"
             ".status { font-size: 24px; margin: 10px 0; }\n"
             "</style>\n"// Fim da seção de estilo CSS.
             "</head>\n"// Fim da seção de cabeçalho do HTML.
             "<body>\n" // Início da seção do corpo do HTML.
             "  <script>\n" // Início da seção de script JavaScript.
             "    // recarrega a página inteira após 1 segundo\n"
             "    setTimeout(function() {\n" // Define um temporizador para recarregar a página.
             "      window.location = window.location.pathname;\n" // Recarrega a página atual.
             "    }, 1000);\n" // Define o tempo de espera para 1 segundo.
             "  </script>\n" // Fim da seção de script JavaScript.
             "<h1>Tracking...</h1>\n" // Cabeçalho principal da página.
             "<p class=\"temperature\">Temperatura Interna: %.2f &deg;C</p>\n" // Exibe a temperatura interna formatada.
             "<p class=\"status\">Estado do Botao: %s</p>\n" // Exibe o estado do botão (pressionado ou solto).
             "<p class=\"status\">Direcao do Joystick: %s</p>\n" // Exibe a direção do joystick.
             "</body>\n" // Fim da seção do corpo do HTML.
             "</html>\n",  // Fim do documento HTML.
             temperature, button_status, last_direction);  // Passa os valores de temperatura, estado do botão e direção do joystick para a string formatada.


    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY); // Envia a resposta HTML formatada para o cliente TCP.
    tcp_output(tpcb);  // Garante que os dados sejam enviados imediatamente.

    free(request); // Libera a memória alocada para a requisição recebida.
    pbuf_free(p); // Libera o buffer de pacotes (pbuf) que foi recebido.

    return ERR_OK; // Retorna um código de erro OK, indicando que a operação foi bem-sucedida.
}

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv); // Define a função de callback para receber dados na nova conexão TCP.
    return ERR_OK; // Retorna um código de erro OK, indicando que a operação foi bem-sucedida.
}

// Função principal
int main()
{
    stdio_init_all(); // Inicializa a biblioteca padrão de entrada/saída.

    gpio_init(BUTTON_A); // Inicializa o pino do botão A.
    gpio_set_dir(BUTTON_A, GPIO_IN); // Define o pino do botão A como entrada.
    gpio_pull_up(BUTTON_A); // Ativa o resistor de pull-up interno para o botão A.

    // Inicializa a arquitetura CYW43 (Wi-Fi)
    while (cyw43_arch_init()) // Tenta inicializar a arquitetura CYW43.
    {
        printf("Falha ao inicializar Wi-Fi\n"); // Imprime mensagem de erro se a inicialização falhar.
        sleep_ms(100); // Aguarda 100 ms antes de tentar novamente.
        return -1; // Retorna -1 em caso de falha.
    }

    cyw43_arch_gpio_put(LED_PIN, 0); // Desliga o LED integrado.
    cyw43_arch_enable_sta_mode(); // Habilita o modo station (cliente) do Wi-Fi.

    printf("Conectando ao Wi-Fi...\n"); // Imprime mensagem de conexão ao Wi-Fi.
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000)) // Tenta conectar ao Wi-Fi com timeout de 20 segundos, o tempo pode ser alterado conforme o tempo de conexão da sua rede.
    {
        printf("Falha ao conectar ao Wi-Fi\n"); // Imprime mensagem de erro se a conexão falhar.
        sleep_ms(100); // Aguarda 100 ms antes de tentar novamente.
        return -1; // Retorna -1 em caso de falha.
    }

    printf("Conectado ao Wi-Fi\n"); // Imprime mensagem de sucesso ao conectar ao Wi-Fi.

    if (netif_default) // Verifica se a interface de rede padrão está disponível.
    {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));  // Imprime o endereço IP do dispositivo.
    }

    // Configura o servidor TCP
    struct tcp_pcb *server = tcp_new(); // Cria um novo controle de protocolo TCP.
    if (!server) // Verifica se a criação do servidor foi bem-sucedida.
    {
        printf("Falha ao criar servidor TCP\n"); // Imprime mensagem de erro se a criação falhar.
        return -1; // Retorna -1 em caso de falha.
    }

    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) // Tenta associar o servidor TCP à porta 80.
    {
        printf("Falha ao associar servidor TCP à porta 80\n"); // Imprime mensagem de erro se a associação falhar.
        return -1; // Retorna -1 em caso de falha.
    }

    server = tcp_listen(server); // Coloca o servidor em modo de escuta.
    tcp_accept(server, tcp_server_accept); // Define a função de callback para aceitar conexões TCP.

    printf("Servidor ouvindo na porta 80\n"); // Imprime mensagem indicando que o servidor está ouvindo.

    // Inicializa o ADC
    adc_init(); // Inicializa o ADC.
    adc_set_temp_sensor_enabled(true); // Habilita o sensor de temperatura no ADC.

    while (true) // Loop principal.
    {
        cyw43_arch_poll(); // Realiza a verificação de eventos da arquitetura CYW43.
    }

    cyw43_arch_deinit(); // Desativa a arquitetura CYW43 (Wi-Fi).
    return 0; // Retorna 0, indicando que o programa terminou com sucesso.
}