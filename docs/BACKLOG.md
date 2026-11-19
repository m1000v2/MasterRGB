# Backlog do MasterRGB

Anotações de melhorias aprovadas para as próximas versões. Cada item vira uma tarefa
concreta quando a versão for aberta.

## 0.4.4 — suporte a plugins oficiais (Plugin API v5)

- [ ] **Backport da Plugin API v5**: atualizar `OpenRGBPluginInterface.h` para usar o IID
  oficial `"org.openrgb.OpenRGBPluginInterface"` e `OPENRGB_PLUGIN_API_VERSION 5`.
- [ ] **Camada OpenRGBPluginAPI**: implementar `OpenRGBPluginAPIInterface` (`OpenRGBPluginAPI.h` e
  `OpenRGBPluginAPI.cpp`) no núcleo do MasterRGB para intermediar a comunicação com as DLLs.
- [ ] **Controladores virtuais**: adicionar `RGBController_Virtual` para suportar plugins como
  Visual Map, Hardware Sync e E1.31 Receiver.
- [ ] **Validação com as DLLs**: testar carregamento dos plugins enviados na aba Configurações > Plugins.

## 0.4.3 — remoção da aba Informações

- [x] **Remover a aba Informações**: completamente eliminada da barra de abas principal.
- [x] **Dados migrados para Apoiar**: versão do app, versão do Qt, versão do Windows,
  licença GPL-2.0, direitos autorais e links agrupados no painel "Sobre o MasterRGB".

## 0.4.2 — correções do perfil e ícone

- [x] **Restauração do perfil salvo**: removida a restrição de `--startminimized` e adicionada
  espera pela detecção de hardware para aplicar aos dispositivos.
- [x] **Regressão do ícone**: restaurado o ícone correto com margens (`assets/MasterRGB-icon-0.3.0-bom.ico`).

## 0.4.1 — ajustes rápidos

- [x] **Ícone da área de trabalho**: o `.exe` agora carrega o "M" em `assets/MasterRGB-icon.ico`
  e `assets/MasterRGB-icon.png` (0.4.1).
- [x] **Confirmar botão Verificar atualizações**: testado e operacional com as releases no GitHub.

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
