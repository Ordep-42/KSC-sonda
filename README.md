# KSC-Sonda

## Plataforma de instrumentação atmosférica de baixo custo baseada no conceito CanSat.

Projeto de desenvolvimento de uma sonda compacta para aquisição, processamento e transmissão de parâmetros ambientais e geofísicos. A plataforma integra sensores digitais, receptor GNSS e um sistema de telemetria sem fio, utilizando um microcontrolador STM32 como unidade central de processamento.

### Visão geral

A KSC-Sonda foi desenvolvida para aplicações de instrumentação atmosférica em experimentos com balões meteorológicos. Seu projeto prioriza baixo custo, modularidade, autonomia operacional e eficiência energética.

O sistema realiza a aquisição periódica de variáveis ambientais e geofísicas, associa as medições às informações de posicionamento e altitude e transmite os dados para uma estação de recepção em solo.

### Características principais

- Aquisição de dados ambientais e geofísicos.
- Posicionamento e determinação de altitude por GNSS.
- Telemetria digital em radiofrequência.
- Protocolo binário próprio para transmissão de dados.
- Aquisição dos sensores a 10 Hz.
- Transmissão de telemetria a 1 Hz.
- Arquitetura embarcada com estratégias de redução do consumo energético.
- Estrutura compacta baseada no conceito CanSat.

### Hardware

| Componente | Função |
|-------|-------|
| STM32F103C8T6 | Unidade central de processamento |
| BMP280 | Pressão atmosférica |
| AHT10 | Temperatura e umidade relativa |
| QMC5883L | Magnetômetro de três eixos |
| NEO-7M | Posicionamento GNSS |
| EBYTE E220-900T22D | Transceptor de telemetria |
| 3 × AAA | Alimentação da plataforma |
| Conversor Boost | Elevação da tensão de alimentação |

### Firmware

O firmware é desenvolvido em linguagem C para o microcontrolador STM32F103C8T6, operando com clock de 64 MHz.

Suas principais responsabilidades são:

- Inicialização e gerenciamento dos periféricos.
- Aquisição periódica dos sensores.
- Recepção e interpretação dos dados GNSS.
- Organização das informações em pacotes binários.
- Gerenciamento da comunicação sem fio.
- Controle dos estados operacionais e otimização energética.

A aquisição dos sensores ambientais e geofísicos ocorre em intervalos de 100 ms (10 Hz), enquanto a transmissão dos pacotes de telemetria é realizada a cada segundo (1 Hz).

### Ferramentas e versões

STM32CubeMX | 6.18.0
STM32CubeF1 Firmware Package | 1.8.7

Configuração do projeto

- MCU: STM32F103C8T6
- Arquitetura: ARM Cortex-M3
- Frequência do sistema: 64 MHz
- Linguagem: C
- Compilador: ARM GCC
- Configurações: Debug e Release

O arquivo "sondinha.ioc" contém as configurações de hardware e periféricos utilizadas na geração do projeto, permitindo consultar e reproduzir a configuração do microcontrolador no STM32CubeMX. A frequência principal utiliza o HSI, devido a instabilidades com o cristal externo.

### Protocolo de comunicação

A comunicação utiliza um protocolo binário próprio, desenvolvido para organizar os dados adquiridos e transmiti-los de maneira estruturada.

O datagrama contém campos de identificação, controle e sequência, além dos dados de telemetria, incluindo:

- Informações do sistema.
- Dados de posicionamento GNSS.
- Parâmetros ambientais.
- Componentes do campo magnético.

O campo de sequência permite identificar e ordenar os pacotes recebidos, auxiliando na análise da continuidade da comunicação.

### Estação de solo

A estação de recepção utiliza um segundo transceptor EBYTE E220-900T22D conectado a um conversor UART-USB, permitindo a comunicação com computadores e dispositivos compatíveis.

### Alimentação e eficiência energética

A plataforma utiliza três pilhas alcalinas AAA conectadas em série, associadas a um conversor elevador de tensão para disponibilizar 5 V ao sistema.

O firmware incorpora estratégias de redução do consumo energético, incluindo gerenciamento dos períodos de atividade e utilização de modos de baixo consumo do microcontrolador, para obter 5h30 de operação continua.

### Resultados experimentais

Os ensaios realizados incluem testes individuais dos sensores, verificação do posicionamento GNSS, avaliação do consumo energético e testes de comunicação sem fio.

Foi comprovado experimentalmente um alcance mínimo de comunicação de 3 km em condições de visada direta, mesmo em ambiente urbano.

### Desenvolvimento futuro

- Integração de novos sensores ambientais e geofísicos.
- Instrumentação para detecção de eventos ionizantes.
- Medição de radiação ultravioleta.
- Implementação de sistema de recuperação da sonda.
- Registro local de dados em cartão SD.
- Realização de experimentos com balões meteorológicos.

### Licença

A licença e as condições de utilização e distribuição do projeto devem ser consultadas no arquivo "LICENSE" deste repositório.
