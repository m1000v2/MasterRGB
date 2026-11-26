# MasterRGB 0.4.5

## O que mudou nesta versão

- **Ativação e desativação em tempo real de plugins corrigida**: resolvido o problema em que clicar na caixa de seleção de um plugin não o inicializava nem criava suas abas de interface. O carregador agora inicializa o plugin, seus componentes visuais e o menu da bandeja imediatamente.
- **Criação dinâmica de abas de plugins**: ao ativar um plugin (como Visual Map, Hardware Sync, Scheduler, etc.), sua respectiva aba é criada instantaneamente na barra de navegação correspondente. Ao desativar, a aba é removida de forma limpa.
- **Persistência bidirecional de estado**: a ativação e desativação dos plugins agora é sincronizada e persistida tanto no formato padrão do OpenRGB (`Plugins`) quanto nas configurações do MasterRGB (`PluginManager`), mantendo os plugins ativados após fechar e reabrir o aplicativo.
- **Deduplicação automática de plugins**: plugins presentes tanto na pasta do executável quanto na pasta de usuário (`%APPDATA%\OpenRGB\plugins`) não aparecem mais duplicados na lista.
- **Proteção contra exceções**: adicionado tratamento de exceções robusto na carga e descarga de plugins de terceiros para evitar encerramento inesperado do programa.

# MasterRGB 0.4.4

## O que mudou nesta versão

- **Suporte completo a plugins oficiais (Plugin API v5)**: adicionado suporte ao identificador oficial `org.openrgb.OpenRGBPluginInterface` e à especificação API v5 do OpenRGB 1.0, permitindo carregar, habilitar e configurar diretamente os plugins oficiais.
- **Camada de ponte e adaptador (OpenRGBPluginAPI)**: implementação da interface de comunicação para plugins com suporte a controladores virtuais (`RGBController_Virtual`), mapas de LED, sincronização de hardware e perfis.
- **Base WinRing0 100% preservada**: os drivers de acesso direto ao barramento SMBus/I2C permanecem na versão estável WinRing0, garantindo o funcionamento perfeito e sem conflitos da placa de vídeo PowerColor RX 6700 XT e dos módulos de memória ENE DRAM.
- **Compatibilidade testada**: suporte aos plugins oficiais OpenRGB Visual Map, Hardware Sync, Scheduler, HTTP Hook, E1.31 Receiver e Skin Plugin.

# MasterRGB 0.4.3

## O que mudou nesta versão

- **Aba "Informações" eliminada**: a aba redundante e visualmente inconsistente foi completamente removida da barra superior.
- **Informações do sistema migradas para "Apoiar"**: versão do app, versão da base OpenRGB, versão do Qt, sistema operacional, licença GPL-2.0, direitos autorais e repositório GitHub agora ficam integrados em um painel limpo e padronizado na aba "Apoiar".
- **Botões integrados**: botões de "Verificar atualizações" e "Abrir no GitHub" posicionados diretamente no painel de informações, com suporte a rolagem responsiva.

# MasterRGB 0.4.2

## O que mudou nesta versão

- **Restauração do perfil salvo na inicialização corrigida**: o perfil agora é restaurado tanto
  na abertura normal quanto ao iniciar minimizado com o Windows. A rotina aguarda a detecção
  completa dos dispositivos de hardware terminar antes de enviar as cores aos controladores.
- **Sincronização na aba Início**: o seletor de perfil e o status no painel Início são
  atualizados automaticamente com o perfil restaurado.
- **Ícone com margem correta**: restaurado o ícone oficial sem cortes nas bordas para atalho,
  executável e instalador.

# MasterRGB 0.4.1

## O que mudou nesta versão

- **Ícone novo**: o "M" com a onda colorida substitui o ícone herdado da base. Vale para
  o executável, atalho da área de trabalho, barra de tarefas, Alt+Tab, instalador e
  desinstalador, com versão 0.4.1.0 nos detalhes do arquivo.
- Nenhuma mudança funcional em relação à 0.4.0.

# MasterRGB 0.4.0

## O que mudou nesta versão

- **Identidade completa**: o executável agora carrega o ícone do MasterRGB nos recursos
  do Windows, com versão 0.4.0.0 nos detalhes do arquivo. Atalho da área de trabalho,
  barra de tarefas, Alt+Tab, instalador, desinstalador e janela usam o mesmo ícone.
- **Início virou painel de verdade**: mostra os dispositivos detectados e permite
  escolher e aplicar um perfil salvo sem sair da tela. Os atalhos que só repetiam as
  abas foram removidos.
- **Apoiar com PayPal**: link de doação configurado e QR Code gerado no próprio
  aplicativo, com o aviso de que a doação é voluntária.
- **Atualização dentro do app**: o MasterRGB consulta as releases públicas do GitHub e
  avisa quando existe versão nova, com botão para baixar. Nada é baixado ou instalado
  sem confirmação.
- **Desinstalação com escolha**: o desinstalador pergunta se os perfis e configurações
  devem ser mantidos (Sim) ou removidos junto com o aplicativo (Não).
- **Marca corrigida**: título da janela, mensagem de detecção, links, rótulo do GitHub
  e descrição na aba Informações deixaram de dizer OpenRGB. A atribuição à base
  OpenRGB (GPL-2.0) permanece na página Sobre e no NOTICE.md.
- **Versão em um lugar só**: `patches/source/qt/MasterRGBVersion.h` alimenta o
  executável, o ZIP e o instalador.

## Base

- Motor OpenRGB na branch `release_candidate_1.0rc2wr0` (GPL-2.0).
- Detecção PowerColor Red Devil RX 6700 XT e quatro XPG Spectrix D60G preservadas.

## Ainda pendente

- Modo de efeitos animados (vídeo/GIF) com o aplicativo minimizado: planejado para a 0.5.0.
- Perfis continuam em `%APPDATA%\OpenRGB` para não quebrar configurações existentes.
- Validar em máquina limpa e no hardware de referência.
