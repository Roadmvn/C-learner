# Cours 03 : Pointeurs de Fonctions

## Déclaration

```c
// Syntaxe : type_retour (*nom_pointeur)(types_paramètres)
int (*operation)(int, int);  // Pointeur vers fonction (int, int) → int

// Avec typedef (plus lisible)
typedef int (*Operation)(int, int);
Operation op;
```

## Utilisation

```c
int addition(int a, int b) { return a + b; }
int multiplication(int a, int b) { return a * b; }

int main(void) {
    int (*op)(int, int);

    op = addition;          // Assigner (pas besoin de &)
    printf("%d\n", op(5, 3));  // 8

    op = multiplication;
    printf("%d\n", op(5, 3));  // 15

    return 0;
}
```

## Callbacks

```c
// Fonction qui prend un callback
void traiter_tableau(int *arr, size_t len, int (*transform)(int)) {
    for (size_t i = 0; i < len; i++) {
        arr[i] = transform(arr[i]);
    }
}

int doubler(int x) { return x * 2; }
int carre(int x) { return x * x; }

int main(void) {
    int arr[] = {1, 2, 3, 4, 5};

    traiter_tableau(arr, 5, doubler);  // {2, 4, 6, 8, 10}
    traiter_tableau(arr, 5, carre);    // {4, 16, 36, 64, 100}

    return 0;
}
```

## Tableau de Pointeurs de Fonctions

```c
typedef void (*CommandHandler)(void);

void cmd_help(void) { printf("Aide\n"); }
void cmd_quit(void) { printf("Quitter\n"); exit(0); }
void cmd_status(void) { printf("Status OK\n"); }

CommandHandler commands[] = {
    cmd_help,
    cmd_quit,
    cmd_status
};

// Appel par index
commands[0]();  // Appelle cmd_help
```

## Application : Machine à États

```c
typedef void (*StateHandler)(int event);

void state_idle(int event);
void state_running(int event);
void state_error(int event);

StateHandler current_state = state_idle;

void state_idle(int event) {
    if (event == EVENT_START) {
        printf("Démarrage...\n");
        current_state = state_running;
    }
}

void state_running(int event) {
    if (event == EVENT_ERROR) {
        current_state = state_error;
    }
}

// Boucle principale
void main_loop(void) {
    while (1) {
        int event = get_event();
        current_state(event);  // Appelle le handler courant
    }
}
```

## Application Sécurité : Hooks

```c
// Hook pour intercepter des fonctions
typedef int (*OriginalFunc)(const char *);

OriginalFunc original_puts = NULL;

int hooked_puts(const char *str) {
    printf("[HOOK] Intercepté: %s\n", str);
    return original_puts(str);  // Appeler l'original
}

// Installation du hook (simplifié)
void install_hook(void) {
    original_puts = puts;  // Sauvegarder
    // Rediriger puts vers hooked_puts (via PLT/GOT)
}
```

## Application Sécurité : Vtables

Les objets C++ utilisent des vtables (tables de pointeurs de fonctions) :

```c
// Simulation d'une vtable en C
typedef struct {
    void (*speak)(void *self);
    void (*move)(void *self);
} AnimalVTable;

typedef struct {
    AnimalVTable *vtable;
    char name[32];
} Animal;

void dog_speak(void *self) {
    Animal *a = (Animal*)self;
    printf("%s: Woof!\n", a->name);
}

void cat_speak(void *self) {
    Animal *a = (Animal*)self;
    printf("%s: Meow!\n", a->name);
}

// Vtables
AnimalVTable dog_vtable = { dog_speak, NULL };
AnimalVTable cat_vtable = { cat_speak, NULL };

// Usage
Animal dog = { &dog_vtable, "Rex" };
Animal cat = { &cat_vtable, "Felix" };

dog.vtable->speak(&dog);  // Rex: Woof!
cat.vtable->speak(&cat);  // Felix: Meow!
```

**Exploitation** : Écraser un pointeur de vtable permet de rediriger l'exécution !
