What is this?
=

This is a small project to stress test Godot serialization process.

This project currently support `2.1`, `3.0`, and `master` branch.

There are 3 modes of operation:

- Test (tester.tscn)
- Listen (listner.tscn)
- Send (sender.tscn)

Test
--
Create at least one variant for almost any Godot base class, plus some object. For each of those, encode them, perform permutations on the resulting bytes (including but not limited to, known metric bytes), and try to decode those permutations.

At each step:

- Print test number
- Print the seed (which can also be provided via `--seed INT`) used to generate random permutations
- Print the data if decoding was succesfull.

You can also run a specific test via `--idx INT`. Remember to use the same seed if you want the same result. In this case the original data, and the mutated data will be also shown.

Listen
--

Wait for incoming UDP data on port 1025, tries to decode the packets, shows any valid decoded data.

Send
--

Generate random packets and send them to `127.0.0.1:1025`.

Packet size goes from 1 to `MAX_SIZE`.

For each size, `PERMUTATIONS` permutations are performed as defined in `res://script/sender.gd`


Motivation
-----

As Godot becomes more and more popular, and people starts using it to make online games, network security is becoming more and more important.

For this reason, after discovering a security issue in the Godot serialization functions, I decided to do a full audit of those functions and write this tests to help ensuring a proper fix, and to keep the bar high in future updates.
