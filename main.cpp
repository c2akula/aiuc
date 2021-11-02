#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>
#include <optional>
#include <stack>

constexpr auto MAX = 100;

struct Flight
{
        std::array<char, 20> from{}, to{};
        bool                 skip{}; // used in backtracking
        int                  distance{};

  public:
        Flight() = default;

        Flight(const std::pair<std::string_view, std::string_view> from_to, int distance) : distance{distance},
                                                                                            skip{false}
        {
                const auto[f, t] = from_to;
                // make copies of the arguments
                f.copy(from.data(), from.size());
                t.copy(to.data(), to.size());
        }

        auto operator==(const std::pair<std::string_view, std::string_view> f) const
        {
                const auto[_from, _to] = f;
                return (_from.compare(from.data()) == 0) && (_to.compare(to.data()) == 0);
        }
};

template <size_t N>
class FlightDB
{
        std::vector<Flight> db;
        std::stack<Flight>  bt;

  public:
        enum class SearchMethod
        {
                BreadthFirst,
                DepthFirst,
        };

  public:
        FlightDB()
        { db.reserve(N); }

        /**
         * @brief Appends a pair of cities into the database.
         */
        auto append_flight(const std::pair<std::string_view, std::string_view> from_to, int distance) -> void
        {
                db.emplace_back(from_to, distance);
        }

        /**
         * @brief Given a pair of cities, will find and print the path to follow along with the total distance.
         */
        template <SearchMethod M>
        auto route(const std::pair<std::string_view, std::string_view> from_to) -> void
        {
                search_flight<M>(from_to);
                auto dist = 0;
                while (!bt.empty())
                {
                        const auto t = bt.top();

                        printf("%s to ", t.from.begin());

                        dist += t.distance;
                        bt.pop();
                }
                printf("%s\n", from_to.second.begin());
                printf("Distance is %d\n", dist);
        }

  private:
        /**
         * @brief Determines if there is a flight between two cities.
         * @return nullopt if no flight exists or the distance between the two cities.
         */
        auto get_distance(const std::pair<std::string_view, std::string_view> from_to) -> std::optional<int>
        {
                const auto res = std::find_if(db.begin(),
                                              db.end(),
                                              [&](const auto& f) { return f == from_to; });

                return (res != db.end()) ? std::make_optional(res->distance) : std::nullopt;
        }

        /**
         * @brief When given a city, `find_connecting` searches the database for a connecting flight.
         * @param from name of the city of origin
         * @return name of the destination city and its distance
         * @warning If a connecting flight is found, the connection's skip field is set
         * so as to control backtracking from deadends
         */
        auto find_connecting(const std::string_view from) -> std::optional<std::pair<std::string_view, int>>
        {
                for (auto& f: db)
                {
                        if ((from.compare(f.from.data()) == 0) && !f.skip)
                        {
                                f.skip = true;
                                return std::make_optional(std::make_pair(f.to.begin(), f.distance));
                        }
                }
                return std::nullopt;
        }

        /**
         * @brief The database is searched for a flight between \param from and \param to. If there is a flight, then
         * the routine pushes this connection to the stack and returns. If there is no flight, a search is made for
         * a flight from \param from to anyplace else. If there is, then this connection is pushed to the stack and
         * continues searching until all such connections are found. If there isn't, backtracking takes place by
         * removing the previous connection from the stack and search is resumed.
         * @param from
         * @param to
         * @return
         */
        template <SearchMethod M>
        auto search_flight(const std::pair<std::string_view, std::string_view> from_to) -> void
        {
                if constexpr (M == SearchMethod::DepthFirst)
                {
                        // see if destination is reached
                        if (const auto dist = get_distance(from_to).value_or(0); dist > 0)
                        {
                                bt.emplace(from_to, dist);
                                return;
                        }
                }
                else
                {
                        while (true)
                        {
                                const auto[from, to] = from_to;
                                const auto res = find_connecting(from);
                                if (const auto[anywhere, dist] = res.value(); dist > 0)
                                {
                                        const auto     any_to = std::make_pair(anywhere, to);
                                        if (const auto d      = get_distance(any_to).value_or(0); d > 0)
                                        {
                                                bt.emplace(from_to, dist);
                                                bt.emplace(any_to, d);
                                                return;
                                        }
                                }
                        }
                }


                // try another connection
                const auto[from, to] = from_to;
                if (const auto dist = find_connecting(from); dist)
                {
                        const auto[anywhere, d] = dist.value();
                        bt.emplace(from_to, d);
                        search_flight<M>(std::make_pair(anywhere, to));
                }
                else
                {
                        // backtrack
                        Flight f = bt.top();
                        bt.pop();
                        search_flight<M>(std::make_pair(f.from.begin(), f.to.begin()));
                }
        }

        auto search_results() -> std::optional<std::stack<Flight>>
        { !bt.empty() ? std::make_optional(bt) : std::nullopt; }
};


int main()
{
        using flight_db_t = FlightDB<MAX>;
        flight_db_t flights;

        // setup
        flights.append_flight({"New York", "Chicago"}, 1000);
        flights.append_flight({"Chicago", "Denver"}, 1000);
        flights.append_flight({"New York", "Toronto"}, 800);
        flights.append_flight({"New York", "Denver"}, 1900);
        flights.append_flight({"Toronto", "Calgary"}, 1500);
        flights.append_flight({"Toronto", "Los Angeles"}, 1800);
        flights.append_flight({"Toronto", "Chicago"}, 500);
        flights.append_flight({"Denver", "Urbana"}, 1000);
        flights.append_flight({"Denver", "Houston"}, 1500);
        flights.append_flight({"Houston", "Los Angeles"}, 1500);
        flights.append_flight({"Denver", "Los Angeles"}, 1000);


        flights.route<flight_db_t::SearchMethod::DepthFirst>({"New York", "Los Angeles"});


        return 0;
}
