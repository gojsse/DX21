import { useState } from 'react'
import type { CSSProperties } from 'react'
import { useStore } from '../state/store'
import { auditionNote } from '../audio/useAudio'
import { BANK_NAMES, BANK_CATSEQ, CATS, BANKS, LIB_TAGS } from '../state/seed'
import { panelCard, sectionLabel, paramLabel, keyBtn, keyABtn, segOnSm, spin } from '../theme/tokens'

export function LibraryView() {
  const audioStub = useStore((s) => s.audioStub)
  const [tag, setTag] = useState('ALL')
  const [bank, setBank] = useState(0)
  const [slot, setSlot] = useState(6)

  return (
    <div style={{ display: 'grid', gridTemplateColumns: '216px 1fr', gap: 10, padding: '12px 14px' }}>
      {/* left: banks + drop zone */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
        <div style={{ ...panelCard, padding: '10px 10px 12px', display: 'flex', flexDirection: 'column', gap: 8 }}>
          <div style={sectionLabel}>BANKS</div>
          <div style={{ display: 'flex', alignItems: 'center', gap: 6, background: 'var(--inset)', border: '1px solid var(--panel-b)', borderRadius: 4, padding: '5px 8px' }}>
            <div style={{ font: "400 10px 'IBM Plex Mono'", color: 'var(--faint)' }}>⌕ SEARCH PATCHES…</div>
          </div>
          <div style={{ display: 'flex', flexWrap: 'wrap', gap: 4 }}>
            {LIB_TAGS.map((t) => {
              const on = t === tag
              const st: CSSProperties = on
                ? { ...segOnSm, letterSpacing: '.12em', cursor: 'pointer' }
                : { font: "600 9.5px 'Barlow Condensed'", letterSpacing: '.12em', padding: '3px 9px', borderRadius: 3, color: 'var(--silk)', background: 'var(--panel)', border: '1px solid var(--panel-b)', cursor: 'pointer' }
              return <div key={t} style={st} onClick={() => setTag(t)}>{t}</div>
            })}
          </div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
            {BANKS.map((b, i) => {
              const on = i === bank
              return (
                <div key={i} onClick={() => setBank(i)} style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', padding: '7px 10px', borderRadius: 4, cursor: 'pointer', background: on ? 'var(--szl)' : 'transparent', border: '1px solid ' + (on ? 'var(--szl-b)' : 'transparent') }}>
                  <div style={{ font: "600 11px 'Barlow Condensed'", letterSpacing: '.08em', color: on ? 'var(--acc)' : 'var(--val)' }}>{b[0]}</div>
                  <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>{b[1]}</div>
                </div>
              )
            })}
          </div>
        </div>
        <div onClick={() => audioStub('receive .syx')} style={{ ...panelCard, border: '1px dashed var(--dash)', padding: '14px 12px', display: 'flex', flexDirection: 'column', gap: 6, alignItems: 'center', textAlign: 'center', cursor: 'pointer' }}>
          <div style={{ font: "600 11px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)' }}>DROP A .SYX FILE</div>
          <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)', lineHeight: 1.5 }}>or receive from<br />your synth</div>
        </div>
      </div>

      {/* right: bank grid + transfer */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
        <div style={{ ...panelCard, padding: 12, display: 'flex', flexDirection: 'column', gap: 10 }}>
          <div style={{ display: 'flex', alignItems: 'baseline', justifyContent: 'space-between' }}>
            <div style={{ display: 'flex', alignItems: 'baseline', gap: 10 }}>
              <div style={{ font: "700 15px 'Barlow Condensed'", letterSpacing: '.06em', color: 'var(--ink)' }}>FACTORY A</div>
              <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>30/32 · CLICK TO AUDITION · DRAG TO REORDER</div>
            </div>
            <div style={{ display: 'flex', gap: 3 }}>
              <div onClick={() => audioStub('export .syx')} style={{ ...keyBtn, cursor: 'pointer' }}>EXPORT .SYX</div>
              <div onClick={() => audioStub('new bank')} style={{ ...keyBtn, cursor: 'pointer' }}>NEW BANK</div>
            </div>
          </div>
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(8,1fr)', gap: 6 }}>
            {BANK_NAMES.map((nm, i) => {
              const empty = !nm
              const on = i === slot
              const cs: CSSProperties = { display: 'flex', flexDirection: 'column', gap: 4, padding: '7px 9px', borderRadius: 4, minWidth: 0, cursor: empty ? 'default' : 'pointer', background: on ? 'var(--disp)' : 'var(--panel)', border: '1px solid ' + (on ? 'var(--disp-b)' : 'var(--panel-b)'), boxShadow: on ? 'var(--disp-inset)' : 'none', opacity: empty ? 0.45 : 1 }
              return (
                <div key={i} style={cs} onClick={() => { if (!empty) { setSlot(i); auditionNote() } }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'baseline' }}>
                    <div style={{ font: "500 9px 'IBM Plex Mono'", color: on ? 'var(--disp-dim)' : 'var(--faint)' }}>{(i + 1 < 10 ? '0' : '') + (i + 1)}</div>
                    <div style={{ font: "600 8px 'Barlow Condensed'", letterSpacing: '.12em', color: on ? 'var(--disp-ink)' : (CATS[BANK_CATSEQ[i]] || 'transparent') }}>{BANK_CATSEQ[i]}</div>
                  </div>
                  <div style={{ font: "500 11px 'IBM Plex Mono'", color: on ? 'var(--disp-ink)' : (empty ? 'var(--faint)' : 'var(--ink)'), whiteSpace: 'nowrap', overflow: 'hidden' }}>{empty ? '— — — —' : nm}</div>
                </div>
              )
            })}
          </div>
        </div>

        <div style={{ display: 'grid', gridTemplateColumns: 'auto auto 1fr', gap: 14, alignItems: 'center', ...panelCard, padding: '10px 14px' }}>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 4 }}>
            <div style={paramLabel}>DEVICE Nº</div>
            <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
              <div style={{ ...dispChipSm }}>1</div>
              <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}><div style={spin}>▲</div><div style={spin}>▼</div></div>
            </div>
          </div>
          <div style={{ display: 'flex', gap: 4 }}>
            <div onClick={() => audioStub('send bank')} style={{ ...keyABtn, cursor: 'pointer' }}>SEND BANK TO SYNTH</div>
            <div onClick={() => audioStub('receive bank')} style={{ ...keyBtn, cursor: 'pointer' }}>RECEIVE BANK</div>
          </div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 5, background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 4, padding: '7px 12px', boxShadow: 'inset 0 1px 5px rgba(0,0,0,.3)' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between' }}><div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)' }}>MIDI TRANSMIT? ▸ SENDING VOICE 14/32</div><div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-dim)' }}>44%</div></div>
            <div style={{ height: 5, background: 'var(--disp-track)', borderRadius: 2, overflow: 'hidden' }}><div style={{ height: '100%', width: '44%', background: 'var(--disp-ink)', borderRadius: 2 }} /></div>
          </div>
        </div>
      </div>
    </div>
  )
}

const dispChipSm: CSSProperties = { font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 3, padding: '2px 10px', boxShadow: 'var(--disp-inset)', textAlign: 'center' }
