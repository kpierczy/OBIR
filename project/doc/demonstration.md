# Przebieg demonstracji

1.  Wysłanie CoAP PING
2.  Zapytanie NON GET o zasób /.well-known/core z użyciem tokena
3.  Zapytanie CON GET o zasób /rpn z URI-QUERY 'all'
4.  Zapytanie NON PUT o zasób /rpn z payloadem 'n n * 2 * n 3 * + 7 +'
5.  Zapytanie CON PUT o zasób /rpn z payloadem 'n 6 + n *'
6.  Zapytanie NON GET o zasób /rpn z URI-QUERY 'wyr=1&n=3' (24)
7.  Zapytanie NON GET o zasób /rpn z URI-QUERY 'wyr=2&n=5' (55)
8.  Zapytanie CON GET o zasób /metrics/Waiting_for_ACK (odpowiedź asynchroniczna)
9.  Zapytanie CON GET o zasób /metrics/PUT_inputs (retransmisja)
10. Zapytanie CON GET o zasób /metrics/GET_inputs



# Funkcjonalności

|         Funkcjonalność                        | Pozycja testowa |
| --------------------------------------------- | --------------- |
| Obsługa Tokena                                |       [2]       |
| Obsługa MID                                   |       [2]       |
|                                               |                 |
| Obsługa wiadomości NON (GET i PUT)            |     [2],[4]     |
| Obsługa żadań CON (GET i PUT)                 |     [3],[5]     |
| Obsługa CoAP PING                             |       [1]       |
|                                               |                 |
| Obsługa opcji Content-Format                  |     [1],[2]     |
| Obsługa opcji Uri-Path                        |       [X]       |
| Obsługa opcji Accept                          |        -        |
|                                               |                 |
| Asynchroniczna odpowiedź (długi czas dostępu) |       [9]       |
| Odpowiedź typu CON z retransmisją             |       [8]       |
|                                               |                 |
| Obsługa zasobu /.well-known/core              |       [2]       |
| Obsługa wyrażeń arytmetycznych RPN            |      [4-7]      |
| Obsługa trzech metryk opisujących transmisję  |      [8-10]     |