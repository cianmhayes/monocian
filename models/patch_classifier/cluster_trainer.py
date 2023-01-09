import pickle
from sklearn.cluster import MiniBatchKMeans

class ClusterTrainer(object):
    def __init__(self, n_clusters) -> None:
        self.kmeans = MiniBatchKMeans(n_clusters=n_clusters, compute_labels=False, n_init=8)

    def n_clusters(self) -> int :
        return self.kmeans.n_clusters

    def fit(self, input):
        self.kmeans.partial_fit(input)

    def predict(self, batch):
        return self.kmeans.predict(batch)

    def pickle(self, path):
        with open(path, 'wb') as file:
            pickle.dump(self.kmeans, file)

    def unpickle(self, path):
        with open(path, 'rb') as file:
            self.kmeans = pickle.load(file)
