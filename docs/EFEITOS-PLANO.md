# MasterRGB — Modo de efeitos animados (planejamento 0.5.0)

Este documento descreve o modo "dinâmico" pedido: efeitos tipo vídeo, com o aplicativo
aberto e minimizado. O 0.4.0 entrega apenas o modo estático (perfil aplicado e aplicativo
fechado). Nada aqui foi implementado ainda.

## Dois modos, um aplicativo

| Modo | O que faz | Aplicativo aberto | Consumo |
| --- | --- | --- | --- |
| Estático | Aplica um perfil `.orp` e grava as cores nos controladores | Não | Zero |
| Dinâmico | Lê quadros de um efeito e envia cor por LED continuamente | Sim, minimizado na bandeja | 2% a 8% de CPU |

A escolha fica em **Configurações > Geral > Modo de funcionamento**. No modo estático o
botão fechar encerra o aplicativo; no dinâmico ele minimiza para a bandeja e mostra um
aviso na primeira vez.

## Formatos suportados

- **GIF, WebP animado e APNG** — já decodificados pelo Qt, sem dependência extra.
- **Sequência de PNG/JPG** em uma pasta (`frame_0001.png`, ...) — melhor qualidade.
- **MP4/H.264** — só com FFmpeg embutido; aumenta o instalador em ~80 MB. Fica para uma
  etapa opcional, com aviso de licença (LGPL) e botão para instalar sob demanda.

## Como o vídeo vira cor nos LEDs

1. O quadro é reduzido para uma grade de amostragem (por exemplo, 32×8).
2. Cada dispositivo tem um mapa de LEDs: tira, ventoinha, módulo de memória, etc.
3. Cada LED recebe a cor da célula correspondente do quadro.
4. O motor envia as cores a 20 Hz e chama `UpdateLEDs`.

O mapa é gerado a partir das zonas do OpenRGB, com ajuste manual em uma tela de
calibração (arrastar a tira sobre a prévia do vídeo).

## Regras de desempenho

O próprio aplicativo avisa quando o efeito passa do orçamento:

- Resolução de trabalho máxima de 480×270; o vídeo original pode ser maior.
- 20 quadros por segundo é o teto; 12 já parece fluido em LED.
- Brilho máximo configurável (padrão 60%) para não esquentar fitas e memórias.
- Pausa automática quando um jogo em tela cheia está em foco, ou quando o notebook está
  na bateria.
- Leitura do quadro em uma thread separada; a interface nunca trava.

## Vídeos leves — recomendação para publicar

- Duração de 5 a 15 segundos, em loop.
- 480×270 ou 640×360, 15 fps, WebP animado.
- Fundo escuro e poucas cores grandes rendem mais em LED do que vídeo detalhado.
- Evitar cenas com cortes rápidos: o LED não tem persistência e o efeito fica "piscando".

## Etapas de implementação

1. Motor de efeitos + leitura de GIF/WebP (sem interface).
2. Tela **Efeitos** com prévia, play/pause e seleção de arquivo.
3. Mapa de LEDs por dispositivo e calibração.
4. Minimizar para a bandeja, pausa automática e limite de brilho.
5. Suporte opcional a MP4 com FFmpeg sob demanda.
