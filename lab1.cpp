#include "rpg_engine.h" // Включаємо всі наші класи

/* --------------------- Main demonstration --------------------- */
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)time(nullptr));
    Logger logger(Logger::INFO);

    // Create skill tree (template)
    SkillTree<Skill*> tree;
    tree.generateRandom(randomSkillFactory, 3, 2);
    cout << "Generated Skill Tree (DFS descriptions):\n";
    for (auto& s : tree.descriptionsDFS()) cout << " - " << s << "\n";

    // Create party A
    Party partyA(logger);
    auto w = make_unique<Warrior>("Thorin", logger);
    auto m = make_unique<Mage>("Merlin", logger);
    auto a = make_unique<Archer>("Legolas", logger);

    // equip skills (demonstrate dynamic polymorphism on skills)
    w->equipSkill(unique_ptr<Skill>(new ActiveSkill("Slash", 15)));
    w->equipSkill(unique_ptr<Skill>(new PassiveSkill("Toughness", 8, 0.1)));
    m->equipSkill(unique_ptr<Skill>(new ActiveSkill("Fireball", 20, 12)));
    m->equipSkill(unique_ptr<Skill>(new UltimateSkill("Meteor", 40, 40, 4)));
    a->equipSkill(unique_ptr<Skill>(new ActiveSkill("Piercing Arrow", 12, 6)));

    // add items to inventories (template inventory usage)
    w->getInventory().add(Item("Health Potion", 50));
    m->getInventory().add(Item("Mana Potion", 60));
    a->getInventory().add(Item("Quiver", 20));

    partyA.addMember(move(w));
    partyA.addMember(move(m));
    partyA.addMember(move(a));

    // Create party B
    Party partyB(logger);
    auto w2 = make_unique<Warrior>("Orc1", logger);
    auto m2 = make_unique<Mage>("Witch", logger);
    w2->equipSkill(unique_ptr<Skill>(new ActiveSkill("Cleave", 14)));
    m2->equipSkill(unique_ptr<Skill>(new ActiveSkill("Shadow Bolt", 18)));
    partyB.addMember(move(w2));
    partyB.addMember(move(m2));

    // Show status
    partyA.showStatus();
    partyB.showStatus();
    cout << "Party A combined power: " << partyA.combinedPower() << "\n";
    cout << "Party B combined power: " << partyB.combinedPower() << "\n";

    // Simulate battle
    BattleSimulator sim(logger);
    sim.simulate(partyA, partyB);

    // Demonstrate finding a skill in the skill tree
    auto root = tree.getRoot();
    if (root && root->getSkill()) {
        string q = root->getSkill()->getName();
        auto found = tree.findNodeBySkillName(q);
        if (found) cout << "Found root skill by name: " << q << "\n";
    }

    // Show inventory snapshot
    if (partyA.getMember(0)) {
         cout << "PartyA member 0 inventory: " << partyA.getMember(0)->getInventory().toString() << "\n";
    }

    // End
    cout << "\n--- Demo finished ---\n";
    return 0;
}