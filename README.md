# 📊 Catálogo de Salários dos Servidores Públicos de Gravataí

Este projeto tem como objetivo desenvolver uma aplicação local para importar, organizar e consultar dados salariais dos servidores públicos municipais de Gravataí-RS, a partir de dados abertos disponibilizados pelo Portal da Transparência.

## 👤 Participante
- Nícolas Chaves (projeto individual)

## 🗂 Fonte dos Dados
- [Portal da Transparência da Prefeitura de Gravataí](https://transparencia.gravatai.rs.gov.br/)  
- Formato original: CSV (dados públicos)

---

## 🧾 Descrição do Problema

O sistema desenvolvido visa permitir:
- Importação incremental de dados salariais;
- Armazenamento estruturado e eficiente em arquivos binários;
- Consultas otimizadas por índices;
- Ferramentas de ordenação e análise estatística.

A solução utiliza uma estrutura em C para representar cada servidor público, com suporte a leitura e escrita de arquivos binários e mecanismos de indexação.

---

## 🧱 Estrutura de Dados

```c
struct Servidor {
    char nome[100];
    int matricula;
    float salario_bruto;
    char cargo[100];
    char orgao[100];
    float descontos;
    float salario_liquido;
};
