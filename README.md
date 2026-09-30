# Career Dendrogram System

A console-based career guidance system in simple C++. It profiles a student across 12 interest and aptitude dimensions, ranks 33 careers with an explainable similarity model, and uses **hierarchical agglomerative clustering** to draw a dendrogram that shows where the student sits among career families. That is the idea behind the name.

```
            Business Analyst --------------------+
                                                 +---------------------+
        Chartered Accountant --------------------+                     |
                                                                       |
              Game Developer --------------------------------------+   |
                                                                   |   +--
         Mechanical Engineer ---------+                            |   |
                                      +------------+               |   |
              Civil Engineer ---------+            |               |   |
                                                   +----------+    +---+
  Electronics / VLSI Engi... -------------+        |          |    |
                                          +--------+          |    |
          Aerospace Engineer -----------+ |                   |    |
                                        +-+                   |    |
           Robotics Engineer -----------+                     +----+
                                                              |
          Research Scientist ---------------------------+     |
                                                        |     |
        Quantitative Analyst -----------------+         |     |
                                              +-------+ +-----+
   Machine Learning Engineer ---------------+ |       | |
                                            +-+       | |
              Data Scientist ---------------+         | |
                                                      +-+
       Cybersecurity Analyst -------+                 |
                                    +-----------+     |
     Cloud / DevOps Engineer -------+           |     |
                                                +-----+
                YOUR PROFILE ----------------+  |
                                             +--+
           Software Engineer ----------------+
                             0.00               0.56                 1.12
                             joining distance (smaller = more similar)
```

In the terminal, the path from your profile to the root is highlighted in colour.

---

## Features

| Area | What it does |
|---|---|
| Authentication | Registration and sign-in with salted, iterated SHA-256 hashing (no plain-text passwords), masked password input, a live password-strength meter, and a 60-second lockout after 3 failed attempts. |
| Calibrated assessment | 28 questions in three parts: 18 RIASEC interest items, 6 aptitude self-ratings, and 4 objective quiz questions. Quiz results are blended with self-ratings, and the difference is reported as a **calibration gap** (over- or under-confidence). Supports going back to the previous question and quitting without saving. |
| Explainable recommendations | Ranked matches with fit, salary band and growth outlook. Each career detail shows the traits driving the match, a trait-by-trait "you vs. required" chart, and the **highest-leverage skill**: the single aptitude whose improvement raises fit the most. |
| Career dendrogram | UPGMA clustering of your profile and the careers, rendered as a text tree. It lists your nearest career family and flat families obtained by cutting the tree. |
| What-if simulator | Change any trait and watch the ranking re-order live, with rank movement and fit deltas. Your saved profile is not modified. |
| Roadmap tracker | Track milestone roadmaps for chosen careers, toggle progress, and see the next step. Progress is saved per user. |
| Comparison | Two careers side by side across all 12 dimensions, with your own level alongside. |
| History | Every assessment is kept, so you can see how your profile changes over time. |
| Report export | Plain-text report with profile, top 10 matches and gaps, dendrogram, and roadmap progress. |
| Works everywhere | Plain ASCII boxes and bars that display correctly in any Windows or Linux terminal. Colour can be turned off in Settings or with `--no-color`. |

## The model

**Trait space.** People and careers live in the same 12-dimensional space, normalised to [0, 1]:

| Interests (Holland / RIASEC) | Aptitudes |
|---|---|
| Realistic, Investigative, Artistic, Social, Enterprising, Conventional | Quantitative, Computing, Communication, Design, Life Sciences, Business |

**Fit score.**

```
fit   = 0.65 * shape + 0.35 * level
shape = (pearson(user, career) + 1) / 2              pattern of strengths
level = max(0, 1 - ||user - career|| / (0.5 * sqrt(12)))   absolute closeness
```

Shape alone would rate a uniformly weak profile the same as a uniformly strong one. Level alone over-rewards average profiles. Blending the two avoids both problems.

**Calibration.** For quantitative and computing ability, `trait = 0.5 * self_rating + 0.5 * quiz_score`, and `gap = mean(self_rating - quiz_score)`.

**Clustering.** Agglomerative, average linkage (UPGMA) on Euclidean distance, using the Lance-Williams update. Complexity is O(n^3) for n leaves, which is instant for this dataset. The tree is drawn on a grid of characters with `-`, `|` and `+`.

## How to run

**Dev-C++ (or any IDE):** open `career_dendrogram.cpp` and press **F11** (Compile & Run). No settings are needed, even on Dev-C++ 5.11.

**Command line:**

```bash
g++ career_dendrogram.cpp -o career_dendrogram
./career_dendrogram          # Windows: career_dendrogram.exe
```

Works with any C++ compiler, including old ones in C++98 mode. No external libraries.

Options: `--no-color` turns colours off, `--data <folder>` changes where data is saved.

## Project files

```
career_dendrogram.cpp   the whole program
data/careers.csv        editable list of careers (created automatically if missing)
README.md
```

### How the code is organised

The program is one file, divided into clearly marked sections, one per class or module:

| Section | What it contains |
|---|---|
| Utils | colours, boxes, bars, menus, input, text and time helpers |
| Sha256 | SHA-256 hash for passwords |
| Models | `Career`, `CareerList`, `User`, `AssessmentResult`, `RoadmapProgress` |
| FileStore | class template `FileStore<T>`: saves and loads any record type |
| Auth | `AuthManager`: register, login, lockout, change password |
| Quiz | the 28-question assessment |
| Engine | career scoring (`Match`) and the `Dendrogram` class |
| CareerApp | all the screens |
| main | program start |

The code is written in plain, beginner-friendly C++: simple classes, `for` loops, `vector` and `string`, and short comments.

- **Classes (OOP):** each class has one job. `AuthManager` handles accounts, `CareerList` holds the careers, `Dendrogram` builds and draws the tree, and `CareerApp` shows the screens.
- **Templates:** `FileStore<T>` is one class template used for three kinds of records (users, assessments, roadmaps). `limitValue<T>()` is a small function template that keeps a number within a range.
- **File handling:** every record is one line of text with fields separated by `|`, so the files are easy to read and edit.

## Data files

All files are plain text with one record per line and fields separated by `|`.

| File | Format |
|---|---|
| `data/careers.csv` | `id\|name\|domain\|12 traits (0-10)\|salary low\|salary high\|growth 1-5\|entry exams\|summary\|milestone;milestone;...` |
| `data/users.db` | `username\|salt\|hash\|full name\|created\|failed attempts\|locked until` |
| `data/profiles/<user>/assessments.db` | `timestamp\|12 trait scores\|calibration gap` |
| `data/profiles/<user>/roadmap.db` | `career id\|milestone flags (e.g. 11000)` |

To add a career, append a line to `data/careers.csv`. No recompilation is needed.

## Notes

- Salary ranges (INR lakh per annum) and growth outlooks are indicative and meant for orientation.
- Password hashing uses salted SHA-256 with 5,000 stretching rounds. That is appropriate for a learning project; a production system should use a dedicated KDF such as Argon2 or bcrypt.
