# E-commerce - Estrutura do Banco de Dados

Este repositório contém o esquema SQL, procedures e triggers para um sistema de e-commerce e será submetido para a segunda nota da disciplina de Bando de Dados 2.

## Integrantes do grupo

* `Victor Giuliano de Freitas Silva`
* `Arthur Santos Vieira da Silva` 
* `Caio César Leandro Amorim`
* `Ricardo Francisco Fragoso da Silva`

## Esquema SQL (Tabelas)

Principais entidades do banco de dados:

* **Clientes e Endereços**: Gerenciam dados dos usuários e locais de entrega;
* **Produtos e Categorias**: Organizam o catálogo de itens, incluindo preço e descrição;
* **Pedidos e ItensPedido**: Processam as transações de compra, ligando clientes e produtos;
* **Pagamentos e Avaliações**: Registram pagamentos e o feedback dos clientes sobre os produtos;
* **Fornecedores**: Gerenciam dados dos fornecedores de produtos;
* **LogEstoque**: Tabela de auditoria que rastreia todas as alterações de estoque.

## Stored Procedures

As procedures automatizam as operações de negócio:

* **`AdicionarPedidoCompleto`**: Cria um pedido com múltiplos itens de forma atômida, validando e atualizando o estoque;
* **`AdicionarItemAoPedido`**: Adiciona um produto a um pedido já existente;
* **`AtualizarStatusPedido`**: Altera o status de um pedido (ex: de 'Processando' para 'Enviado');
* **`AdicionarAvaliacaoProduto`**: Insere uma nova avaliação de produto.


## Triggers

Aplicam regras de negócio automaticamente:

* **`tg_impede_estoque_negativo`**: Bloqueia qualquer operação que resulte em estoque negativo;
* **`tg_log_alteracao_estoque`**: Grava um registro na tabela `LogEstoque` sempre que o estoque de um produto é alterado;
* **`tg_atualiza_valor_pago_pedido`**: Atualiza o total pago em um pedido a cada novo pagamento registrado (requer uma coluna `ValorPago` na tabela `Pedidos`).
