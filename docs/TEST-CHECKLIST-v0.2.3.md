# MasterRGB 0.2.3 — Checklist de validação

## Visual e navegação
- [ ] A lista lateral de Informações permanece como subnavegação técnica por dispositivo; o item Sobre o MasterRGB aparece primeiro. Essa lista compartilhada não deve ser removida.
- [ ] A janela e o ícone mostram MasterRGB.
- [ ] Ícones originais dos dispositivos são legíveis (sem glifos truncados ou números soltos).
- [ ] A roda HSV/seletor de cores aparece integralmente.
- [ ] Tema escuro padrão não cobre widgets personalizados.
- [ ] Tema claro alternativo permanece legível.
- [ ] Abas principais Dispositivos, Informações, SDK, Configurações, Visão geral e Apoiar alternam corretamente.
- [ ] Visão geral lista os dispositivos detectados.
- [ ] Abrir controles em cada cartão seleciona o dispositivo correspondente na aba Dispositivos.
- [ ] Em Informações, a página Sobre o MasterRGB aparece no topo; subpáginas por dispositivo permanecem acessíveis.
- [ ] Tela Sobre mostra MasterRGB, versão, Qt real e nome do Windows (sem placeholders).

## Apoiar e links
- [ ] A aba Apoiar está presente na janela principal (não apenas no launcher).
- [ ] Campo PayPal permite salvar link HTTPS paypal.com / paypal.me.
- [ ] Link incorreto é rejeitado.
- [ ] Após salvar, o link persiste ao reiniciar.
- [ ] Botão Apoiar via PayPal abre o endereço configurado.
- [ ] Acesso GitHub abre a URL configurada.
- [ ] O destino é o mesmo do YT Desk Monitor, conferido pelo usuário antes de publicação Stable.

## Hardware
- [ ] Quatro XPG Spectrix D60G detectadas.
- [ ] Efeitos RAM Static, Rainbow e Breathing funcionam.
- [ ] PowerColor Red Devil RX 6700 XT detectada.
- [ ] Efeitos da GPU continuam aplicando.
- [ ] A alteração de cor da GPU é testada separadamente em Static.
- [ ] A MSI MPG B550 Gaming Plus continua detectada.
- [ ] G502 e G915 continuam detectados.

## Perfis e inicialização
- [ ] Salvar perfil .orp e aplicar.
- [ ] Selecionar um perfil `.orp`, aplicar no núcleo e confirmar o registro em `MasterRGB_LastProfile.txt`.
- [ ] Fechar/abrir MasterRGB e confirmar que o último perfil carregado é aplicado.
- [ ] Reiniciar o Windows com a tarefa habilitada e confirmar o mesmo comportamento.
- [ ] Conferir `%APPDATA%\MasterRGB\startup.log` e a linha `CONFIRMADO` do carregamento.
- [ ] Início com Windows elevado requer UAC e cria tarefa MasterRGB Startup.
- [ ] Tarefa inicia minimizado e aplica perfil após logon.
- [ ] Desativar início remove a tarefa.
- [ ] Desinstalador remove a tarefa.

## Segurança e publicação
- [ ] Não desativar Defender, Integridade da Memória ou bloqueios de drivers.
- [ ] Relatório de diagnóstico é local e revisado antes de compartilhar.
- [ ] Testar portable em caminho permanente e instalador em máquina limpa.
- [ ] Não publicar Stable enquanto qualquer teste crítico estiver pendente.


## Regressão 0.2.3
- [ ] Barra de título mostra MasterRGB, inclusive após 10 segundos.
- [ ] Aba lateral "Sobre o MasterRGB" não exibe nome do produto-base no título.
- [ ] Configurações > Aparência troca Escuro/Claro imediatamente.
- [ ] O tema escolhido permanece após fechar e abrir novamente.
- [ ] A aba Apoiar mostra acentos corretamente (doação, repositório, atualizações).
- [ ] Atribuições ao motor OpenRGB permanecem na tela Sobre/NOTICE.
