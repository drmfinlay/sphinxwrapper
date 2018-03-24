"""
Classes using GNU aspell to localise between English variants.
"""

from aspell import Speller


class Localiser(object):
    """
    This class uses GNU aspell to localise words to the English spelling for the
    specified language variant - e.g. en_AU, en_UK, en_US.

    Note that the following lenient assumptions are made about words being
    localised:
    - words can be misspelled, but should have the correct number of special
      characters such as apostrophes, hyphens and spaces.
    - words should not be misspelled at the start: i.e. 'bcolor' instead of
      'color'
    """
    def __init__(self, lang):
        """
        :type lang: str
        """
        # Set up an AspellSpeller object
        self.speller = Speller(("lang", lang))

        # Set of words looked up that were incorrect
        self._incorrect_words = set()

        # Cache of suggestions for strings
        self._suggestion_cache = {}

    def localise_word(self, word):
        """
        Localise a single word.
        """
        # Do further checking if the word is not correct
        if word in self._incorrect_words or not self.speller.check(word):
            # Add incorrect words to a set for quick lookup
            self._incorrect_words.add(word)  # sets are distinct

            # TODO It may be better for this function to change the order of suggestions

            def valid(suggestion):
                # We won't have words that are this incorrect from Pocket Sphinx,
                # so don't use use these suggestions.
                if word[0] != suggestion[0]:
                    return False

                # Assume that 'word' has the correct number of special characters
                # E.g. 'col-or' and 'col or' aren't valid suggestions for 'color' to
                # be localised to a non-US English variant
                for c in [" ", "'", "-"]:
                    if word.count(c) != suggestion.count(c):
                        return False

                return True

            # Used cached suggestions where possible
            if word in self._suggestion_cache.keys():
                suggestions = self._suggestion_cache[word]

            # Otherwise get them from the speller, filtered through the
            # validation function.
            else:
                suggestions = list(filter(valid, self.speller.suggest(word)))
                
                # Cache the suggestions for next time
                self._suggestion_cache[word] = suggestions

            if suggestions:
                word = suggestions[0]
        return word

    def localise_words(self, words):
        """
        Localise a string of multiple words using GNU aspell.
        :type words: str
        :rtype: str
        """
        # Split the string into a list of words and localise each word
        words = map(self.localise_word, words.split())

        # Join the words into a string again and return it
        return " ".join(words)
