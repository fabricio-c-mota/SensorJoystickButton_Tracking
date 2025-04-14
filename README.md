# SensorJostickButton_Tracking: Sistema de Monitoramento Via Servidor para a placa BitDogLab, Raspeberry PI Pico W

O presente sistema integra o monitoramento de um botão, a leitura do sensor de temperatura embutido e a captura da direção do joystick (interpretada de acordo com a “Rosa dos Ventos”) via conexão Wi-Fi. O firmware roda em uma placa BitDogLab com Raspberry Pi Pico W e utiliza o protocolo lwIP para comunicação TCP/HTTP.

## Configuração e Execução

### Descrição e Funcionamento

O projeto foi estruturado para executar duas atividades principais:

- **Monitoramento do Botão e Leitura de Temperatura:**  
  O código verifica se o botão físico (conectado ao GP5) está **"Pressionado"** ou **"Solto"** e, a cada 1 segundo, envia essa informação para um servidor web. Em paralelo, é realizada a leitura do sensor de temperatura interno da placa BitDogLab, convertendo a leitura do ADC em graus Celsius.

- **Leitura do Joystick e Determinação da Direção:**  
  As leituras analógicas do joystick (eixos X e Y) são interpretadas para determinar a direção conforme a “Rosa dos Ventos” (Norte, Sul, Leste, Oeste, Nordeste, Sudeste, Noroeste e Sudoeste).

A resposta gerada é uma página HTML que se atualiza automaticamente a cada segundo, exibindo a temperatura interna, o status do botão e a direção do joystick.

### Estrutura de Arquivos e Pastas

O projeto está organizado de forma modular, seguindo uma estrutura padrão para desenvolvimento com Raspberry Pi Pico W:

- **`lwipopts.h`:**  
  Contém as definições e configurações mínimas para o funcionamento do lwIP, como habilitação de TCP/UDP, DHCP, DNS, HTTP e parâmetros de memória.

- **`CMakeLists.txt`:**  
  Configura as bibliotecas e inclui diretórios essenciais do Pico SDK e do lwIP (por exemplo, `pico_stdlib`, `hardware_gpio`, `hardware_adc` e `cyw43_arch`). Além disso, relaciona os arquivos para o servidor HTTP (como `httpd.c` e `fs.c`).

- **Código Fonte Principal (`SensorJoystickButton_Tracking.c` ou equivalente):**  
  Implementa a lógica central do projeto. Nele são configuradas as inicializações de hardware, a conexão Wi-Fi, os ADCs para o sensor de temperatura e o joystick, o monitoramento do botão e a estruturação da resposta HTML para o servidor TCP.

> **Índice de Diretórios:**  
> - `/include`: Contém o arquivo `lwipopts.h`.  
> - `/src`: Contém o `SensorJoystickButton_Tracking.c` (ou arquivo principal) e os arquivos de implementação do servidor HTTP.  
> - `CMakeLists.txt`: Localizado na raiz do projeto.

## Especificações de Hardware

- **Placa Utilizada:** BitDogLab com Raspberry Pi Pico W.
- **Componentes Conectados:**
  - **Botão físico (GP5):** Usado para acionar a leitura do status (pressionado ou solto).
  - **Joystick:**  
    - Eixo X conectado ao GP0.  
    - Eixo Y conectado ao GP1.
  - **Sensor de Temperatura Interno:** Utiliza o ADC para converter valores em graus Celsius.

As conexões foram realizadas utilizando os componentes já integrados à placa, facilitando futuras expansões ou modificações.

## Especificação do Firmware

O firmware integra diversas funcionalidades em um único código:

1. **Inicialização e Configuração:**  
   - Inicialização das bibliotecas padrão do Pico SDK.  
   - Configuração dos ADCs para leitura da temperatura e dos eixos do joystick.  
   - Inicialização do Wi-Fi utilizando a arquitetura **CYW43**, conforme definido em `lwipopts.h`.

2. **Loop Principal e Fluxo de Execução:**  
   - Leitura contínua do status do botão, dos valores do sensor de temperatura e dos eixos do joystick.  
   - A função `get_direction()` interpreta os valores analógicos do joystick, mapeando-os para direções (por exemplo, Norte, Sul, etc.).
   - Geração de uma resposta HTML que consolida as informações coletadas e é enviada ao cliente via conexão TCP na porta 80.

3. **Requisições HTTP:**  
   - Implementação de um servidor HTTP utilizando lwIP para processar as requisições e retornar a página atualizada com os dados do sensor, status do botão e direção do joystick.

## Decisões de Projeto

- **Integração Modular:**  
  As funcionalidades (botão, sensor de temperatura e joystick) foram implementadas em um único código para aproveitar a similaridade de configurações, simplificando a execução e facilitando a manutenção.

- **Expansibilidade:**  
  A estrutura modular permite que futuras funcionalidades ou sensores sejam integrados facilmente, aumentando a flexibilidade do sistema.

- **Eficiência na Comunicação:**  
  A utilização do lwIP com configurações customizadas em `lwipopts.h` garantiu uma integração eficaz dos protocolos TCP/HTTP com a camada de hardware, permitindo atualizações rápidas e precisas dos dados.

## Conclusão
- **Obs.: Ao consultar em mais detalhes a estrutura do código, o mesmo está integralmente comentado para facilitar o entendimento e comprenssão 🚀.**
  
- O seguinte Readme.md sintetiza o escopo geral das funcionalidades do respectivo projeto de maneira resumida, para mais detalhes sobre as especificações e estrutura do código, refêrencias, etc., siga o link do documento original do projeto abaixo:
- Link do Documento: https://drive.google.com/file/d/10k0HdzPHZbZsIhnfF-CzDcBcKbno2f7K/view?usp=sharing
