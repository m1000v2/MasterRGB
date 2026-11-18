# Backlog do MasterRGB

Anotações de melhorias aprovadas para as próximas versões. Cada item vira uma tarefa
concreta quando a versão for aberta.

## 0.4.1 — ajustes rápidos

- [x] **Ícone da área de trabalho**: o `.exe` agora carrega o "M" em `assets/MasterRGB-icon.ico`
  e `assets/MasterRGB-icon.png` (0.4.1). Os atalhos antigos `MasterRGB.exe - Atalho.lnk` e
  `MasterRGB2.exe - Atalho.lnk` ainda apontam para builds anteriores; decidir se serão
  repontados para `C:\Program Files\MasterRGB\MasterRGB.exe` ou removidos pelo usuário.
- [ ] **Remover a aba Informações**: os dados que interessam (versão do app, versão do
  Qt, versão do Windows, link do projeto) passam para a aba **Apoiar**. Avaliar o que
  fazer com a lista lateral de dispositivos e com as subabas de cada controlador.
- [ ] Confirmar o botão **Verificar atualizações** com uma release mais nova publicada.

## 0.5.0 — efeitos animados

Ver `docs/EFEITOS-PLANO.md` para o desenho completo.

- [ ] Motor de efeitos lendo GIF/WebP animado e sequência de PNG/JPG.
- [ ] Tela **Efeitos** com prévia, play/pause e seleção de arquivo.
- [ ] Mapa de LEDs por dispositivo, com calibração visual.
- [ ] Modo minimizado na bandeja, pausa automática em jogo em tela cheia e na bateria.
- [ ] Limite de brilho para proteção térmica.
- [ ] MP4 opcional via FFmpeg sob demanda (avaliar tamanho e licença).
- [ ] Guia de vídeos leves: 480x270, 15 fps, 5 a 15 s, fundo escuro.

## Ideias em avaliação

- Nomes amigáveis para dispositivos repetidos ("Memória 1" a "Memória 4" em vez de
  quatro "ENE DRAM").
- Instância única, para duas cópias abertas não brigarem pelo driver.
- Abrir direto no último dispositivo usado, opcionalmente.
