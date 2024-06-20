package main

import (
    "context"
    "fmt"
    "go.mongodb.org/mongo-driver/mongo"
    "go.mongodb.org/mongo-driver/mongo/options"
)

func main() {
    // Imposta la stringa di connessione
    connectionString := "mongodb://localhost:27017"

    // Crea un nuovo client
    client, err := mongo.NewClient(options.Client().ApplyURI(connectionString))
    if err != nil {
        panic(err)
    }

    // Connetti il client al server
    err = client.Connect(context.Background())
    if err != nil {
        panic(err)
    }

    // Verifica la connessione
    err = client.Ping(context.Background(), nil)
    if err != nil {
        panic(err)
    }

    fmt.Println("Connesso a MongoDB!")
}