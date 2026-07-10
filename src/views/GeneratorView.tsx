import type { CSSProperties } from 'react'
import { useStore } from '../state/store'
import { GEN_CATS, GEN_NAMES } from '../state/seed'
import { panelCard, sectionLabel, keyABtn, segOnF } from '../theme/tokens'
import { Slider } from '../components/Slider'
import { EnvelopeGraph } from '../components/EnvelopeGraph'

export function GeneratorView() {
  const gen = useStore((s) => s.gen)
  const setGenField = useStore((s) => s.setGenField)
  const toggleKeep = useStore((s) => s.toggleKeep)
  const undo = useStore((s) => s.undo)
  const audioStub = useStore((s) => s.audioStub)

  return (
    <div style={{ display: 'grid', gridTemplateColumns: '230px 1fr', gap: 10, padding: '12px 14px' }}>
      {/* left controls */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
        <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 9 }}>
          <div style={sectionLabel}>CATEGORY</div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
            {GEN_CATS.map((c, i) => {
              const on = i === gen.category
              const st: CSSProperties = on
                ? { ...segOnF, textAlign: 'left', padding: '6px 11px', font: "600 11px 'Barlow Condensed'", letterSpacing: '.1em', cursor: 'pointer' }
                : { padding: '6px 11px', borderRadius: 3, font: "600 11px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--val)', background: 'var(--inset)', border: '1px solid var(--panel-b)', cursor: 'pointer' }
              return <div key={c} style={st} onClick={() => setGenField('category', i)}>{c}</div>
            })}
          </div>
        </div>
        <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 12 }}>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 6 }}>
            <div style={{ display: 'flex', justifyContent: 'space-between' }}><div style={{ font: "600 10px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)' }}>COUNT</div><div style={{ font: "500 12px 'IBM Plex Mono'", color: 'var(--ink)' }}>{gen.count}</div></div>
            <Slider value={gen.count} min={1} max={16} onChange={(v) => setGenField('count', v)} />
          </div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 6 }}>
            <div style={{ display: 'flex', justifyContent: 'space-between' }}><div style={{ font: "600 10px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)' }}>MUTATION</div><div style={{ font: "500 12px 'IBM Plex Mono'", color: 'var(--ink)' }}>{gen.mutation}%</div></div>
            <Slider value={gen.mutation} min={0} max={100} onChange={(v) => setGenField('mutation', v)} />
          </div>
          <div onClick={() => audioStub('generate bank')} style={{ padding: '9px 0', textAlign: 'center', borderRadius: 4, cursor: 'pointer', font: "700 12px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)', boxShadow: '0 2px 3px rgba(0,0,0,.3)' }}>GENERATE</div>
          <div onClick={() => undo()} style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)', textAlign: 'center', cursor: 'pointer' }}>UNDO GENERATE · ⌘Z</div>
        </div>
      </div>

      {/* results tray */}
      <div style={{ ...panelCard, padding: 12, display: 'flex', flexDirection: 'column', gap: 10 }}>
        <div style={{ display: 'flex', alignItems: 'baseline', justifyContent: 'space-between' }}>
          <div style={{ display: 'flex', alignItems: 'baseline', gap: 10 }}>
            <div style={{ font: "700 15px 'Barlow Condensed'", letterSpacing: '.06em', color: 'var(--ink)' }}>{GEN_CATS[gen.category]} · RESULTS TRAY</div>
            <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>{GEN_NAMES.length} CANDIDATES · CLICK TO AUDITION</div>
          </div>
          <div onClick={() => audioStub('export bank .syx')} style={{ ...keyABtn, cursor: 'pointer' }}>EXPORT BANK AS .SYX</div>
        </div>
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4,1fr)', gap: 8 }}>
          {GEN_NAMES.map(([name], i) => {
            const kept = gen.kept[i]
            return (
              <div key={i} style={{ display: 'flex', flexDirection: 'column', gap: 7, background: kept ? 'var(--szl)' : 'var(--inset)', border: '1px solid ' + (kept ? 'var(--szl-b)' : 'var(--panel-b)'), borderRadius: 5, padding: '9px 10px' }}>
                <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                  <div style={{ font: "600 11px 'Barlow Condensed'", letterSpacing: '.06em', color: kept ? 'var(--acc)' : 'var(--ink)' }}>{name}</div>
                  <div style={{ width: 6, height: 6, borderRadius: '50%', background: 'var(--acc)', boxShadow: '0 0 6px var(--accg)' }} />
                </div>
                <EnvelopeGraph ep={{ a: (i % 4) * 0.18 + 0.05, d1: (i % 3) * 0.25 + 0.15, s: ((i + 1) % 4) * 0.24, r: (i % 2) * 0.3 + 0.15 }} w={150} h={30} />
                <div style={{ display: 'flex', gap: 4 }}>
                  <div onClick={() => toggleKeep(i)} style={{ cursor: 'pointer', font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.1em', padding: '2px 8px', borderRadius: 3, color: kept ? 'var(--keyA-ink)' : 'var(--key-ink)', background: kept ? 'var(--keyA)' : 'var(--key)', border: '1px solid ' + (kept ? 'var(--keyA-b)' : 'var(--key-b)') }}>{kept ? 'KEPT ✓' : 'KEEP'}</div>
                  <div onClick={() => audioStub('discard ' + name)} style={{ cursor: 'pointer', font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.1em', padding: '2px 8px', borderRadius: 3, color: 'var(--silk)', border: '1px solid var(--panel-b)' }}>DISCARD</div>
                </div>
              </div>
            )
          })}
        </div>
      </div>
    </div>
  )
}
