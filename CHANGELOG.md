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
