{-# OPTIONS_GHC -Wall #-}
{-# LANGUAGE  FlexibleInstances #-}
module Waterfall(waterfall, mkNode,Node(Node),Edge(Edge),mkEdge,Mergeable(union,unions),getRegion,getEdges,getNode,getWeight,size) where


data Node a = Node a [Edge a]
data Edge a = Edge Int (Node a)


-- mergeChildren applied to an edge returns the edge after merging;
-- the boolean value indicates whether or not the edge contained
-- a regional minimum amongst its children.

class Mergeable a where
  union :: a -> a -> a
  unions :: [a] -> a

-- waterfall creates a (hypothetical) edge into the root of the MST
-- and then proceeds to call the main mergeChildren function.


waterfall :: Mergeable a => Node a -> Node a
waterfall (Node r []) = Node r []
waterfall (Node r es) = (getNode.fst.mergeChildren) (Edge  (1 + maximum (map getWeight es)) (Node r es))

-- f was the initial mergeChildren function (now known as mergeChildren)
--
-- f :: Mergeable a => Edge a -> (Edge a,Bool)
-- f n@(Edge w (Node r [])) = (n,False)
-- f (Edge w (Node r es) ) =
--   let ((e,b):es') = sortBy cmpEdge (map f es) in
--   let w' = getWeight e in
--     case (b==True && (w'<=w)) of
--     True   ->  (join r ((e,False): es') w, True)
--     False  ->  (join r ((e,b): es')  w, flag (w'>=w))


-- mergeChildren walks the tree bottom-up and labels each edge
-- with True or False depending on whether the child has been joined
-- with a regional minimum below it (see also long explanation below)
--
-- it also calls the join function when regions need merging.
{-# SECIALIZE mergeChildren :: Edge (Set a) -> (Edge (Set a),Bool)#-}
{-# SECIALIZE mergeChildren :: Edge (Heap a) -> (Edge (Heap a),Bool)#-}
mergeChildren :: (Mergeable a )=> Edge a -> (Edge a,Bool)
mergeChildren n@(Edge _ (Node _ [])) = (n,False)
mergeChildren (Edge w (Node r es) )
  | (b && w'<=w) = (join r ((e,False): es') w, True)
  | otherwise    = (join r ((e,b): es')  w, (w'<w))
  where
    ((e,b),es') = minEdge es
    w' = getWeight e

minEdge :: (Mergeable a)  => [Edge a] -> ((Edge a,Bool),[(Edge a,Bool)])
minEdge [] = error "the impossible happened"
minEdge es = let x:xs = map mergeChildren es in minEdge' (x,[]) xs

minEdge' ::  ((Edge a,Bool),[(Edge a,Bool)]) -> [(Edge a,Bool)] -> ((Edge a,Bool),[(Edge a,Bool)] )
minEdge' = foldr (\x (e,es)-> case (cmpEdge x e) of
  LT -> (x,e:es)
  _ -> (e,x:es))

-- Initial join function
-- (should recurse only once because the tree is built bottom-up)
--
-- join ::  Mergeable a=> a -> [(Edge a,Bool)] -> Int -> Edge a
-- join r [] w = Edge w (Node r [])
-- join r ((Edge v n,b):es) w
--  | b   =        Edge w' (Node r' ((Edge v n ):es'))
--  | otherwise  = Edge w' (Node r''  es'')
--   where
--     r'' = (union r' (getRegion n))
--     es'' = (es' ++ getEdges n)
--     Edge w' (Node r' es') = join r es w



-- join function, with fold instead of recursion
--
-- r is the region in the current node;
-- w is the weight of the edge into the current node;
-- ebs are the (bool-labelled) edges from the current node to its children;
--
-- select the False-labelled edges, find the nodes at the end of them and
-- merge r into the regions of those nodes; this gives a bigger region, newr;
--
-- take the children (edges) of those nodes, into es, and make them
-- children of the current node (whose region was r);
--
-- the newly-created node has an edge of (unchanged) weight w coming into it,
-- and is made of the new region, newr;
-- as edges it has all the old True-labelled edges, as well as the edges
-- of the children who have merged into r.



join ::  Mergeable a=> a -> [(Edge a,Bool)] -> Int -> Edge a
join r es w | r `seq` es `seq` w `seq` False = undefined
join r ebs w =
   (Edge w (Node newr es))
   where
     (rs, es) = extractEdgeRegions ebs [] []
     newr = unions (r:rs)

-- Prepare for absorbing the regions in the children of a node
-- by putting the region of the current node(s) in a list (as),
-- and the regions of the children in a list (bs).

extractEdgeRegions :: [(Edge a,Bool)] ->[a] ->  [Edge a] -> ([a],[Edge a])
extractEdgeRegions xs as rs  | xs `seq` as `seq` rs `seq` False = undefined
extractEdgeRegions [] as rs = (as,rs)
extractEdgeRegions ((e,True):es) as rs = extractEdgeRegions es as (e:rs)
extractEdgeRegions ((Edge _ n,False):es) as rs = extractEdgeRegions es (getRegion n : as) (getEdges n ++ rs)



-- Auxiliary functions


-- Compare the weights of two edges.
-- Returns a comparison operator that can then be used in sort.

cmpEdge :: (Edge a,Bool) -> (Edge a,Bool) -> Ordering
cmpEdge ((Edge w1 _),b1) ((Edge w2 _),b2)
  | w1 == w2 = compare b2 b1
  | otherwise = compare w1 w2


-- The node that en edge points to

getNode :: Edge a -> Node a
getNode (Edge _ n) = n

-- The weight of an edge

getWeight :: Edge a -> Int
getWeight (Edge w _) = w

-- The region in a node

getRegion :: Node a -> a
getRegion (Node r _) = r

-- The list of edges originating from a node

getEdges :: Node a-> [Edge a]
getEdges (Node _ es) = es

-- The children nodes of a node

--getChildren :: Node a-> [Node a]
--getChildren n = map getNode (getEdges n)

-- Make a node from a region and some edges

mkNode :: Mergeable a =>a -> [Edge a]->  Node a
mkNode a es = Node a es

-- Make an edge of given weight, which points to a node

mkEdge :: Mergeable a => Node a -> Int -> Edge a
mkEdge n v = Edge v n


instance Mergeable [Char] where
  union a b  = a ++ b
  unions = foldr union ""
---- Stuff for testing ----------


  {- f (a.k.a. mergeChildren):

To combine all the correct edges: From a given Node, with weight w
leading to it, we first apply f to each of its chlidren to get the
correctly merged sub-Nodes along with a boolean value representing
whether or not that child has been merged with a regional minimum (RM)
below it.

If the Node is a leaf, then we're done. It is not merged with anything
below it.

If none of the children where merged with a RM then it is safe to
merge with all the children since this won't bring two RM together. If
w is greater than any of the weights leading to the children then our
Node has been merged with a RM.

Assume that some of the children merged with a RM. We still want to
merge with every child that did not merge with a RM since doing so
will not join any RM but we may also want to merge with one of the
children that did since it may have greatest claim over the current
Node. A child will have this claim if the weight leading to it is less
than the weights of all other edges leading to the curretn Node.

The Node we're at merged with a RM if it

 - merged with one of its children that had merged

or if

 - any of its children that did not merge with a RM formed a RM
 themselves.  This relies on the same condition as before: that w is
 not smaller than the weight of any edge leading away from the Node.

-}


instance  Show (Node a) where
  show t = show' 1 (Edge 0 t) ++"\n"

show' :: Int -> Edge a -> [Char]
show' _ (Edge k (Node _ [])) = show k++"->"++ show "!"
show' n (Edge k (Node _ ts)) = show k++ " -> " ++ show "!"   ++ (concat$map (( ("\n"++(f  n "    "))++).(show' (n+1))) ts)
  where
    f 0 _ = ""
    f x t = t ++ f (x-1) t



size :: Node a -> Int
size (Node _ es) =  1+ sum (map (size.getNode) es)
