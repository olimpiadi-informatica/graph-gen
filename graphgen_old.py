#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# graph-gen - https://github.com/olimpiadi-informatica/graph-gen
# Copyright © 2011-2012 Giorgio Audrito <giorgio.audrito@gmail.com>
# Copyright © 2014 William Di Luigi <williamdiluigi@gmail.com>
# Copyright © 2014 Gabriele Farina <gabr.farina@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


""" Libreria di funzioni sui grafi."""

from random import sample, shuffle, choice, randint as _randint
from math import sqrt
from blist import sortedset
from bisect import bisect_right

# random da A a B pero' con variabili long
def randint(A, B=None):
    """Random da A a B che funziona con valori long."""
    if B is None:
        return _randint(0, A-1)
    return _randint(A, B-1)


# campiono M elementi tra 0..N-1, con N eventualmente long, ordinati crescenti; aggiungendo alla Pop
def lsample(N, M, Pop = []):
    """Ritorna una lista ordinata di len(Pop)+M elementi tra 0 e N-1 (eventualmente long)."""
    if not (0 <= M and M + len(Pop) <= N):
        raise StandardError("Parameter M out of bounds.")
    l = [randint(N-M+1) for i in xrange(M)]
    l.sort()
    for i in xrange(M):
        l[i] += i
    if len(Pop) == 0:
        return l
    i = j = 0
    while j < M:
        if Pop[i] > l[j]+i:
            l[j] += i
            j+=1
        else:
            i+=1
    return Pop + l


class graph:
    """
    Classe astratta graph, viene subclassata da ugraph (grafo non diretto)
    e da dgraph (grafo diretto).

    costruttore:
       graph(G[,w])
           grafo copia di G
       graph(N[,w])
           grafo vuoto con N nodi
       graph(N,E[,w])
           grafo con lista di archi assegnata
       graph(N,[M=...],type='graph_type')
           grafo di tipo graph_type con N nodi, se type = 'forest' allora
           M=numero di archi.

    print:
       stampa in formato olimpico randomizzando l'ordine degli archi.
       se w era stato specificato, aggiunge dei pesi a ogni arco
       ottenendoli tramite w().

    confronti:
       i grafi si confrontano con le relazioni insiemistiche sugli archi
       (sottoinsieme...).

    container:
       l'iterazione su un oggetto graph e' sulla lista ordinata dei suoi
       archi.

    operazioni tra grafi:
       +    unione disgiunta (o aggiunta di un arco)
       *    prodotto cartesiano
       &    intersezione
       |    unione
       ~    complementare
       -    trasposto (solo grafi diretti)

    funzioni random:
       shuffle()
           permuta i label dei nodi del grafo
       connect()
           aggiunge il minimo numero di archi necessario per connettere il
           grafo
       addedges(K, [candidates])
           aggiunge K archi random (tra i candidates, oggetto edgerange)

    varie:
       N()
           quantita' di nodi
       M()
           quantita' di archi
       add(e)
           aggiunge un arco
       discard(e)
           rimuove un arco

    Implementa il concetto di grafo allo scopo di generare grafi pseudo-casuali con proprieta' fissate.
    Il grafo viene rappresentato come insieme ordinato di archi.

    Il costruttore consente di creare un qualsiasi tipo speciale noto di grafi, le usuali operazioni aritmetiche consentono di effettuare analoghe operazioni combinatoriali, mentre le usuali operazioni logiche consentono di effettuare le analoghe operazioni insiemistiche.

    E' inoltre possibile aggiungere archi a caso (con il metodo addedges) o aggiungere archi in modo da connettere il grafo (con il metodo connect)."""

    # costruttore
    def __init__(self, N=0, E=None, M=None, w=None, type=None):
        """Costruisce un grafo vuoto con N vertici, e insieme di archi E (se specificato).
        Se type e' specificato, costruisce invece un grafo di quel tipo.
        I valori ammissibili per type sono cycle,path,tree,forest,clique.
        Se w e' specificato, il grafo viene considerato pesato, con pesi generati dalla funzione w().
        E' anche ammessa l'instanziazione graph(G) con G un grafo gia' esistente."""
        if isinstance(N,graph) and E is None:
            E = [self.cod(e) for e in N]
            N = N.V
        if not ((E is None) or (type is None)):
            raise StandardError("Incompatible parameters specified.")
        self.V=N
        self.w=w
        if E and isinstance(E[0],list):
            E=sortedset([self.cod(e) for e in E])
        if E is None:
            E=sortedset([])
        if len(E)==0 and N > 1:
            if type is 'cycle':
                for i in xrange(N):
                    E.add(self.cod([i,(i+1)%N]))
            if type is 'path':
                for i in xrange(N-1):
                    E.add(self.cod([i,(i+1)%N]))
            if type is 'tree':
                for i in xrange(1,N):
                    E.add(self.cod([randint(i),i]))
            if type is 'forest':
                if not (0 <= M < N):
                    raise StandardError("Parameter M out of bounds.")
                for i in lsample(N-1,M):
                    E.add(self.cod([randint(i+1),i+1]))
            if type is 'clique':
                for i in xrange(N-1):
                    for j in xrange(i+1,N):
                        E.add(self.cod([i,j]))
            if type is 'star':
                for i in xrange(1,N):
                    E.add(self.cod([0,i]))
            if type is 'wheel':
                for i in xrange(1,N):
                    E.add(self.cod([0,i]))
                    E.add(self.cod([i,(i+1)%N]))
        # eventualmente aggiungere: gear, caterpillar/lobster, BIPARTITE
        self.E = sortedset(E)
    # funzioni di stampa
    def __repr__(self):
        """Rappresentazione python del grafo."""
        return self.__class__.__name__ + '(' + str(self.V) + ',' + str([e for e in self]) + ')'
    def __str__(self):
        """Rappresentazione olimpica del grafo."""
        s = str(self.N()) + ' ' + str(self.M()) + '\n'
        Ed = list(self.E)
        shuffle(Ed)
        for e in Ed:
            de = self.dec(e)
            s += str(de[0]+1) + ' ' + str(de[1]+1)
            if self.w is not None:
                s += ' ' + str(self.w())
            s += '\n'
        return s.rstrip()
    def printedges(self, zero_based = False):
        """Rappresentazione olimpica del grafo, senza la prima riga."""
        s = ""
        Ed = list(self.E)
        shuffle(Ed)
        for e in Ed:
            de = self.dec(e)
            s += str(de[0]+(0 if zero_based else 1)) + ' ' + str(de[1]+(0 if zero_based else 1))
            if self.w is not None:
                s += ' ' + str(self.w())
            s += '\n'
        return s.rstrip()
    # funzioni di confronto
    def __lt__(self,other):
        """Relazione di sottoinsieme proprio."""
        return self.E < other.E
    def __le__(self,other):
        """Relazione di sottoinsieme."""
        return self.E <= other.E
    def __eq__(self,other):
        """Relazione di uguaglianza degli archi."""
        return self.E == other.E
    def __ne__(self,other):
        """Relazione di disuguaglianza degli archi."""
        return self.E != other.E
    def __gt__(self,other):
        """Relazione di sovrainsieme proprio."""
        return self.E > other.E
    def __ge__(self,other):
        """Relazione di sovrainsieme."""
        return self.E >= other.E
    # funzioni da container
    def __len__(self):
        """Numero di archi del grafo."""
        return len(self.E)
    def __getitem__(self,i):
        """Restituisce l'i-esimo arco del grafo."""
        return self.dec(self.E[i])
    def __iter__(self):
        """Restituisce un iteratore sugli archi del grafo."""
        return _generic_iter(self)
    def __contains__(self,e):
        """Verifica se un arco e' presente o meno nel grafo."""
        if isinstance(e, list):
            e = self.cod(e)
        return (e in self.E)
    # unione disgiunta di grafi (+)
    def __add__(self,other):
        """Unione disgiunta di grafi."""
        G = self.__class__(self.V,self.E)
        G += other
        return G
    def __iadd__(self,other):
        """Unione disgiunta di grafi."""
        if isinstance(other,graph):
            self.E |= [self.cod([e[0]+self.V,e[1]+self.V]) for e in other]
            self.V += other.V
        else:
            self.add(other)
        return self
    # prodotto cartesiano di grafi (*)
    def __mul__(self,other):
        """Prodotto cartesiano di grafi."""
        G = self.__class__()
        for i in xrange(other.V):
            G += self
        for e in other:
            for i in xrange(self.V):
                G += [e[0]*self.V+i,e[1]*self.V+i]
        return G
    def __imul__(self,other):
        """Prodotto cartesiano di grafi."""
        G = self * other
        self.V = G.V
        self.E = G.E
        return self
    # intersezione di grafi (&)
    def __and__(self,other):
        """Intersezione di grafi."""
        G = self.__class__(self.V,self.E)
        G &= other
        return G
    def __iand__(self,other):
        """Intersezione di grafi."""
        self.V = min(self.V,other.V)
        self.E = self.E & other.E
        return self
    # unione di grafi (|)
    def __or__(self,other):
        """Unione di grafi."""
        G = self.__class__(self.V,self.E)
        G |= other
        return G
    def __ior__(self,other):
        """Unione di grafi."""
        self.V = max(self.V,other.V)
        self.E = self.E | other.E
        return self
    # grafo complementare (~)
    def __invert__(self):
        """Grafo complementare."""
        G = self.__class__(self.V,xrange(self.mMax()) - self.E)
        return G
    # funzioni astratte di codifica degli archi.
    # devono rispettare che:
    #   *  0 <= cod(e) < mMax()
    #   *  cod(e) = cod(e') => e = e'
    #   *  cod(e) e' indipendente dal self
    #   *  gli archi validi per V=N sono esattamente mMax()
    def cod(self, e):
        """Codifica un arco in un intero univoco e indipendente da N,M tra 0 e mMax()."""
        raise NotImplementedError("Abstract class graph must be inherited.")
    def dec(self, n):
        """Decodifica l'id di un arco."""
        raise NotImplementedError("Abstract class graph must be inherited.")
    def mMax(self):
        """Il numero massimo di archi che un grafo con N nodi puo' contenere."""
        raise NotImplementedError("Abstract class graph must be inherited.")
    # calcolo della taglia
    def N(self):
        """Restituisce il numero di nodi del grafo."""
        return self.V
    def M(self):
        """Restituisce il numero di archi del grafo."""
        return len(self.E)
    # aggiunta e rimozione di un arco
    def add(self,e):
        """Aggiunge un arco al grafo."""
        if max(e[0],e[1]) >= self.V:
            self.V = max(e[0],e[1])+1
        self.E.add(self.cod(e))
    def discard(self,e):
        """Rimuove un arco dal grafo, se presente."""
        self.E.discard(self.cod(e))
    # aggiungo K nuovi archi a caso, tra i candidati (oggetto edgerange-style)
    def addedges(self, K, candidates = None):
        """Aggiunge K archi a caso al grafo, tra i candidati (oggetto di tipo edgerange o set/list di archi)."""
        if candidates is None:
            self.E.add(self.mMax())
            new = lsample( self.mMax() - self.M() + 1, K)
            i = j = 0
            while j < K:
                if self.E[i] > new[j]+i:
                    new[j] += i
                    j+=1
                else:
                    i+=1
            self.E.remove(self.mMax())
            self.E |= new
        else:
            dup = sortedset([])
            for e in self:
                if e in candidates:
                    dup.add(self.cod(e))
            new = lsample( len(candidates) - len(dup), K)
            dup.add(self.mMax())
            i = j = 0
            while j < K:
                if dup[i] > candidates[new[j]+i]:
                    new[j] = candidates[new[j]+i]
                    j+=1
                else:
                    i+=1
            self.E |= new
    # aggiunge archi fino a connettere il grafo
    def connect(self):
        """Aggiunge il minor numero di archi necessario a connettere il grafo."""
        lbl = range(self.N())
        rnk = [1 for i in lbl]
        def find(x):
            if x == lbl[x]:
                return x
            lbl[x] = find(lbl[x])
            return lbl[x]
        def union(x, y):
            lx = find(x)
            ly = find(y)
            if lx == ly:
                return
            if rnk[lx]  < rnk[ly]:
                lx, ly = ly, lx
            lbl[ly] = lx
            rnk[lx] += rnk[ly]
        for e in self:
            union(e[0],e[1])
        comp = [ [] for i in range(self.N())]
        for i in range(self.N()):
            comp[lbl[i]].append(i)
        comp = filter(lambda x: len(x) > 0, comp)
        shuffle(comp)
        kcomp = [ len(i) for i in comp ]
        for i in range(1,len(kcomp)):
            kcomp[i] += kcomp[i-1]
        for i in range(1,len(comp)):
            a = randint(kcomp[i-1])
            a = bisect_right(kcomp, a) # upper bound
            a = choice(comp[a])
            b = choice(comp[i])
            self.add((a,b))
    # permuta i nodi del grafo
    def shuffle(self):
        """Permuta casualmente i nodi del grafo tra di loro."""
        lbl = range(self.V)
        shuffle(lbl)
        new = [self.cod([lbl[e[0]],lbl[e[1]]]) for e in self]
        self.E = sortedset(new)


class ugraph(graph):
    """Implementazione della classe graph per grafi non diretti."""
    def cod(self, e):
        a=max(e[0],e[1])
        b=min(e[0],e[1])
        return a*(a-1)/2 + b
    def dec(self, n):
        a = int(round(sqrt(2*(n+1))))
        b = n - a*(a-1)/2
        if randint(0,2) == 0:
            return [a,b]
        return [b,a]
    def mMax(self):
        return self.N()*(self.N()-1)/2


class dgraph(graph):
    """Implementazione della classe graph per grafi diretti."""
    def __init__(self, N=0, E=None, M=None, w=None, type=None):
        """Comprende tutti i costruttori per graph, piu' type='dag'.
        Contempla la conversione di un grafo non diretto in grafo diretto, raddoppiando gli archi."""
        if isinstance(N,ugraph) and E is None:
            E = [self.cod(e) for e in N]
            E += [e^1 for e in E]
            N=N.V
        if type is 'dag':
            if M is None:
                M=E
            E = [2*e+1 for e in lsample( N*(N-1)/2, M)]
            type = None
        graph.__init__(self,N,E,M,w,type)
    def cod(self, e):
        a=max(e[0],e[1])
        b=min(e[0],e[1])
        return a*(a-1) + 2*b + (0 if e[0] > e[1] else 1)
    def dec(self, n):
        a = int(round(sqrt(2*(n/2+1))))
        b = n/2 - a*(a-1)/2
        if n%2 == 0:
            return [a,b]
        return [b,a]
    def mMax(self):
        return self.N()*(self.N()-1)
    # grafo trasposto
    def __neg__(self):
        """Restituisce il grafo trasposto."""
        G = self.__class__(self.V)
        G.E = [cod([dec(e)[1],dec(e)[0]]) for e in self.E]
        G.E.sort()
        return G


class dag(graph):
    """Implementazione della classe dag per grafi diretti aciclici."""
    def __init__(self, N=0, E=None, M=None, w=None, type=None):
        self.lbl = None
        graph.__init__(self,N,E,M,w,type)
    def cod(self, e):
        a=max(e[0],e[1])
        b=min(e[0],e[1])
        return a*(a-1)/2 + b
    def dec(self, n):
        a = int(round(sqrt(2*(n+1))))
        b = n - a*(a-1)/2
        return [b,a]
    def mMax(self):
        return self.N()*(self.N()-1)/2
    def __str__(self):
        """Rappresentazione olimpica del grafo."""
        s = str(self.N()) + ' ' + str(self.M()) + '\n'
        Ed = list(self.E)
        shuffle(Ed)
        if self.lbl is None:
            self.lbl = range(self.V)
        for e in Ed:
            de = self.dec(e)
            s += str(self.lbl[de[0]]+1) + ' ' + str(self.lbl[de[1]]+1)
            if self.w is not None:
                s += ' ' + str(self.w())
            s += '\n'
        return s.rstrip()
    def printedges(self):
        """Rappresentazione olimpica del grafo, senza la prima riga."""
        s = ""
        Ed = list(self.E)
        shuffle(Ed)
        for e in Ed:
            de = self.dec(e)
            s += str(self.lbl[de[0]]+1) + ' ' + str(self.lbl[de[1]]+1)
            if self.w is not None:
                s += ' ' + str(self.w())
            s += '\n'
        return s.rstrip()
    # permuta i nodi del grafo
    def shuffle(self, perm = None):
        """Permuta casualmente i nodi del grafo tra di loro."""
        if perm is not None:
            self.lbl = perm
        else:
            self.lbl = range(self.V)
            shuffle(self.lbl)


class edgerange:
    """ edgerange

    Range immutabile di archi, tra blocchi contigui di nodi ad altri blocchi contigui di nodi.
    I blocchi sorgente e destinazione di un range non si possono overlappare.
    E' possibile concatenare diversi range a patto che gli archi di un range siano tutti maggiori di quelli del range precedente.

    L'oggetto non crea fisicamente il range, consentendo un utilizzo efficiente anche con range particolarmente grandi.

    rappresenta un range di archi.
    
    instanziazione:

    edgerange(g,[0,3],[3,6])
        tutti gli archi da 0,1,2 a 3,4,5
    edgerange(g,[[[0,3],[3,6]],[[6,7],[1,5]]])
        come prima piu' quelli da 6 a 1,2,3,4
    edgerange(g,[0,3],[3,6]) + edgerange([6,7],[1,5])
        come prima

    utilizzo:

    r = edgerange(...)
    for i in edgerange(...)
    if  e in edgerange(...)
    temp = r[i] (sconsigliato)
    
    for e in edgerange(g,[0,3],[1,5]) + edgerange([6,7],[1,5]):
    ...
    g.addedges(3,edgerange(g,[0,3],[1,5]))
    """

    # costruttore
    def __init__(self, edge_type, ran, rdest = None):
        """Costruisce un oggetto edgerange, con archi come in un grafo edge_type."""
        if not isinstance(edge_type,graph):
            if not ( issubclass(edge_type,graph) ):
                raise StandardError("Parameter 1 (edge_type) must be a graph.")
            edge_type = edge_type()
        if rdest is not None:
            ran = [[ran,rdest]]
        for r in ran:
            if not ( r[0][1] <= r[1][0] or r[1][1] <= r[0][0] ):
                raise StandardError("Self-overlapping range.")
        for r in xrange(len(ran)-1):
            if not ( edge_type.cod([ran[r][0][1]-1,ran[r][1][1]-1]) < edge_type.cod([ran[r+1][0][0],ran[r+1][1][0]]) ):
                raise StandardError("Overlapping ranges.")
        self.e_t = edge_type
        self.R = ran
        self.L = [(r[0][1]-r[0][0])*(r[1][1]-r[1][0]) for r in ran]
        self.LL = sum(self.L)
    def __repr__(self):
        """Rappresentazione python dell'oggetto."""
        return self.__class__.__name__ + '(' + self.e_t.__class__.__name__ + ',' + str(self.R) + ')'
    # funzioni da container
    def __len__(self):
        """Dimensione del range."""
        return self.LL
    def __getitem__(self,i):
        """Restituisce l'i-esimo arco nel range."""
        if not (0 <= i < self.LL):
            raise IndexError("Index out of bound.")
        n=0
        while i-self.L[n]>=0:
            i -= self.L[n]
            n += 1
        if self.R[n][0][1] <= self.R[n][1][0]:
            a = i%(self.R[n][0][1] - self.R[n][0][0]) + self.R[n][0][0]
            b = i/(self.R[n][0][1] - self.R[n][0][0]) + self.R[n][1][0]
        else:
            a = i/(self.R[n][1][1] - self.R[n][1][0]) + self.R[n][0][0]
            b = i%(self.R[n][1][1] - self.R[n][1][0]) + self.R[n][1][0]
        return self.e_t.cod([a,b])
    def __iter__(self):
        """Restituisce un iteratore al range."""
        return _generic_iter(self)
    def __contains__(self,e):
        """Verifica se l'arco e fa parte del range."""
        if not isinstance(e,list):
            e = self.e_t.dec(e)
        for r in self.R:
            if (r[0][0] <= e[0] < r[0][1]) and (r[1][0] <= e[1] < r[1][1]):
                return True
            if (self.e_t.cod([e[1],e[0]]) == self.e_t.cod(e)):
                if (r[0][0] <= e[1] < r[0][1]) and (r[1][0] <= e[0] < r[1][1]):
                    return True
        return False
    # join di ranges
    def __add__(self,other):
        """Concatena due edgerange, se possibile."""
        new = self.__class__(self.e_t,self.R + other.R)
        return new


class _generic_iter:
    """Iteratore su una struttura con indicizzazione a parentesi."""
    def __init__(self,obj):
        self.i = 0
        self.obj = obj
    def __iter__(self):
        return self
    def next(self):
        if self.i >= len(self.obj):
            raise StopIteration()
        self.i += 1
        return self.obj[self.i-1]


if __name__ == "__main__":
    print "Ambiente di test non implementato."
