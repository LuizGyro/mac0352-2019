# Congifuração do firewall

Para adicionar um nova regra para o firewall, abra o arquivo 
fw.json, então adcione:

    "ruleX" : { 
        "protocol": "<UDP ou TCP>",
        "ip_source": "<ip da fonte>",
        "ip_dest": "<ip de destino>",
        "port": "<porta>"
    }

Não é necessário que todos os campos sejam preenchidos, por
exemplo, se só se deseja impedir todos os pacotes vindos do ip
10.0.0.1, basta usar a regra:

    "ruleX" : { 
        "ip_source": "10.0.0.1"
    }

O ip pode ser do tipo IPv4 ou IPv6.

Se for desejado que mais uma porta seja bloqueada, mais de uma regra deve ser usada.
